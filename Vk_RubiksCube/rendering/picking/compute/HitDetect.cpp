#include "HitDetect.h"
#include "../../../utils/ImageUtils.h"
#include "../../../utils/MemoryUtils.h"
#include "../../../utils/RenderUtils.h"
#include "../../../vulkan/DeviceManager.h"
#include "../../../utils/FileUtils.h"
#include "../../../materials/ShaderObject.h"
#include "../../../structs/PushConstantBlock.h"
#include "../../../Config.h"
#include "../../../core/DrawableEntity.h"
#include "../../../core/Engine.h"
#include <glm/gtx/string_cast.hpp>

#include "../../Renderer.h"
#include "../../../platform/WindowManager.h"
#include "../../../structs/Ray.h"
#include "../../../utils/Initializers.h"
#include "../../../structs/TriangleInfo.h"
#include "../../../utils/Vk_Utils.h"

rendering::compute::HitDetect::HitDetect(EngineContext& engine_context): engine_context(engine_context), compute(),
                                                                         readback_compute_buffer(),
                                                                         world_transform_buffer({}),
                                                                         pipeline_cache(nullptr)
{
    device_manager = engine_context.device_manager.get();
    renderer = engine_context.renderer.get();
    swapchain_manager = engine_context.swapchain_manager.get();
    window_manager = engine_context.window_manager.get();
    first_submit_done = false;
}

void rendering::compute::HitDetect::create_readback_buffer()
{
    utils::MemoryUtils::allocate_buffer_with_readback_access(device_manager->get_allocator(),  sizeof(glm::vec4), readback_compute_buffer);
    readback_compute_buffer.buffer_address = utils::MemoryUtils::get_buffer_device_address(engine_context.dispatch_table, readback_compute_buffer.buffer);
}

void rendering::compute::HitDetect::create_ray_buffer()
{
    utils::MemoryUtils::allocate_buffer_with_mapped_access(device_manager->get_allocator(), sizeof(Ray), ray_buffer);
    ray_buffer.buffer_address = utils::MemoryUtils::get_buffer_device_address(engine_context.dispatch_table, ray_buffer.buffer);
}

Ray rendering::compute::HitDetect::create_picking_ray(glm::vec2 mouse_screen_pos, glm::vec2 screen_size, 
                      glm::mat4 view_matrix, glm::mat4 proj_matrix)
{
    // Convert screen coordinates to NDC (-1 to 1)
    float x = (2.0f * mouse_screen_pos.x) / screen_size.x - 1.0f;
    float y = 1.0f - (2.0f * mouse_screen_pos.y) / screen_size.y;
    
    // Create points in NDC space (near and far)
    glm::vec4 ray_start_ndc = glm::vec4(x, y, -1.0f, 1.0f); // Near plane
    glm::vec4 ray_end_ndc = glm::vec4(x, y, 1.0f, 1.0f);    // Far plane
    
    // Transform to world space
    glm::mat4 inv_proj_view = glm::inverse(proj_matrix * view_matrix);
    
    glm::vec4 ray_start_world = inv_proj_view * ray_start_ndc;
    glm::vec4 ray_end_world = inv_proj_view * ray_end_ndc;
    
    // Perspective divide
    ray_start_world /= ray_start_world.w;
    ray_end_world /= ray_end_world.w;
    
    // Create ray
    Ray pick_ray;
    pick_ray.origin = glm::vec3(ray_start_world);
    pick_ray.direction = glm::normalize(glm::vec3(ray_end_world - ray_start_world));
    pick_ray.t_min = 0.001f;
    pick_ray.t_max = glm::length(glm::vec3(ray_end_world - ray_start_world));
    
    return pick_ray;
}

void rendering::compute::HitDetect::create_ray_data() const
{
    Vk_SceneData scene_data = engine_context.renderer->get_scene_data();
    
    Ray pick_ray = create_picking_ray(glm::vec2(window_manager->local_mouse_x /*763*/, window_manager->local_mouse_y /*485*/),
                                        glm::vec2(swapchain_manager->get_swapchain().extent.width, swapchain_manager->get_swapchain().extent.height),
                                        scene_data.view, scene_data.projection);
    
    
    auto allocator = device_manager->get_allocator();
    void* data;
    vmaMapMemory(allocator, ray_buffer.allocation, &data);
    memcpy(data, &pick_ray, sizeof(Ray));
    vmaUnmapMemory(allocator, ray_buffer.allocation);
}

void rendering::compute::HitDetect::update_transforms()
{
    auto& entities = core::Engine::get_drawable_entities();

    for (int i = 0; i < cubies.size(); ++i)
    {
        auto model_matrix = cubies[i]->get_transform()->get_model_matrix();
        world_matrices[i] =  model_matrix;
    }

    VkDeviceSize model_transform_buffer_size = world_matrices.size() * sizeof(glm::mat4);
    utils::MemoryUtils::map_persistent_data(device_manager->get_allocator(), world_transform_buffer.allocation, world_transform_buffer.allocation_info, world_matrices.data(), model_transform_buffer_size);
}

void rendering::compute::HitDetect::init_compute()
{
    utils::RenderUtils::create_command_pool(engine_context, compute.command_pool, vkb::QueueType::compute);
    
    //This will be used to return results of the compute to the CPU
    allocate_command_buffer();

    //Create the compute pipeline we'll be using
    create_compute_pipeline(std::string(SHADER_PATH) + "/object_picker.comp.spv");

    //Load objects in compute shader buffer
    load_objects_in_compute_buffer();

    //Create the readback buffer to read results of the compute shader -> CPU Visible
    create_readback_buffer();

    create_ray_buffer();

    create_compute_fence();

    build_compute_command_buffer();
}

void rendering::compute::HitDetect::allocate_command_buffer()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = compute.command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    auto dispatch_table = engine_context.dispatch_table;
    
    if (dispatch_table.allocateCommandBuffers(&allocInfo, &compute.command_buffer) != VK_SUCCESS)
    {
        // failed to allocate command buffers;
        std::cerr << "failed to allocate command buffers for object picker\n";
    }
}

void rendering::compute::HitDetect::create_compute_fence()
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    engine_context.dispatch_table.createFence(&fenceInfo, nullptr, &fence);
}

void rendering::compute::HitDetect::create_compute_pipeline(const std::string& shader_path)
{
    VkShaderModule shader_module = utils::FileUtils::loadShader(shader_path, engine_context);

    VkPipelineShaderStageCreateInfo shader_stage_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = shader_module,
        .pName = "main",
        .pSpecializationInfo = nullptr
    };

    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(HitTraceConstantBlock);
    
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = initializers::pipelineLayoutCreateInfo(VK_NULL_HANDLE, 0, &push_constant_range, 1);
    engine_context.dispatch_table.createPipelineLayout(&pipeline_layout_create_info, VK_NULL_HANDLE, &compute.pipeline_layout);

    VkComputePipelineCreateInfo compute_pipeline_create_info = initializers::computePipelineCreateInfo(compute.pipeline_layout, 0);
    compute_pipeline_create_info.stage = shader_stage_create_info;

    //Create the pipeline cache
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    engine_context.dispatch_table.createPipelineCache(&pipelineCacheCreateInfo, nullptr, &pipeline_cache);

    engine_context.dispatch_table.createComputePipelines(pipeline_cache, 1, &compute_pipeline_create_info, nullptr, &compute.pipeline);
}

void rendering::compute::HitDetect::load_objects_in_compute_buffer()
{
    auto& entities = core::Engine::get_drawable_entities();

    std::vector<TriangleInfo> vertex_triangles;

    uint32_t world_matrix_insertion_id = 0;
    
    for (const auto& entity : entities)
    {
        //Find only cubies 
        if(auto tag = entity->get_entity_string_id(); tag.find("cubie_entity_") != std::string::npos)
        {
            auto model_matrix = entity->get_transform()->get_model_matrix();
            auto vertices = entity->get_render_data().vertices;
            auto indices = entity->get_render_data().indices;
            auto id = entity->get_entity_id();

            //Track cubies with this array
            cubies.push_back(entity.get());
            
            //Store the world transformation in the vertex info of the triangle 
            world_matrices.push_back(model_matrix);
            ++world_matrix_insertion_id;

            //Get number of triangles from indices
            uint32_t triangle_count = indices.size() / 3;

            for (uint32_t i = 0; i < triangle_count; ++i)
            {
                TriangleInfo triangle_info;
            
                // Get the three vertex indices for this triangle
                uint32_t idx0 = indices[i * 3 + 0];
                uint32_t idx1 = indices[i * 3 + 1];
                uint32_t idx2 = indices[i * 3 + 2];
            
                // Get the actual vertices using the indices
                const Vertex& vert0 = vertices[idx0];
                const Vertex& vert1 = vertices[idx1];
                const Vertex& vert2 = vertices[idx2];
            
                triangle_info.vertex1  =  glm::vec4(vert0.position, 1.0f);
                triangle_info.vertex2 = glm::vec4(vert1.position, 1.0f);
                triangle_info.vertex3 =  glm::vec4(vert2.position, 1.0f);
                triangle_info.normal = glm::vec4(vertices[idx0].normal, 0.0f);
                
                triangle_info.object_id = id;
                triangle_info.world_transform_id = world_matrix_insertion_id;

                vertex_triangles.push_back(triangle_info);
            }
        }
    }

    total_triangle_count = vertex_triangles.size();
    
    VkDeviceSize triangle_info_buffer_size = sizeof(TriangleInfo) * total_triangle_count;
    utils::MemoryUtils::transfer_data_to_gpu(engine_context, compute.command_pool, triangles_buffer, triangle_info_buffer_size, vertex_triangles.data(), triangles_buffer.buffer_address);

    VkDeviceSize model_transform_buffer_size = world_matrices.size() * sizeof(glm::mat4);
    utils::MemoryUtils::allocate_buffer_with_mapped_access(device_manager->get_allocator(), model_transform_buffer_size, world_transform_buffer);
    world_transform_buffer.buffer_address = utils::MemoryUtils::get_buffer_device_address(engine_context.dispatch_table, world_transform_buffer.buffer);
}

void rendering::compute::HitDetect::build_compute_command_buffer()
{
    auto dispatch_table = engine_context.dispatch_table;

    if(first_submit_done)
    {
        dispatch_table.waitForFences(1, &fence, VK_TRUE, UINT64_MAX);
        dispatch_table.resetFences(1, &fence);
    }

    dispatch_table.resetCommandBuffer(compute.command_buffer, 0);

    update_transforms();
    create_ray_data();
    
    //Divide to get the number of triangles
    auto local_size = 64;
    auto group_count_x = (total_triangle_count + local_size - 1) / local_size;

    VkCommandBufferBeginInfo cmd_buffer_begin_info{};
    cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    HitTraceConstantBlock hit_trace_constant_block{};
    hit_trace_constant_block.scene_buffer_addr = renderer->get_gpu_scene_buffer().scene_buffer_address;
    hit_trace_constant_block.world_transform_buffer_addr = world_transform_buffer.buffer_address;
    hit_trace_constant_block.mesh_triangle_buffer_addr = triangles_buffer.buffer_address;
    hit_trace_constant_block.output_buffer_addr = readback_compute_buffer.buffer_address;
    hit_trace_constant_block.ray_buffer_addr = ray_buffer.buffer_address;
    
    dispatch_table.beginCommandBuffer(compute.command_buffer, &cmd_buffer_begin_info);

    dispatch_table.cmdBindPipeline(compute.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipeline);
    dispatch_table.cmdPushConstants(compute.command_buffer, compute.pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(HitTraceConstantBlock), &hit_trace_constant_block);
    dispatch_table.cmdDispatch(compute.command_buffer, group_count_x, 1, 1);

    dispatch_table.endCommandBuffer(compute.command_buffer);
}
