#include "ObjectPicking.h"

#include "../Renderer.h"
#include "../../structs/Vk_ShaderInfo.h"
#include "../../utils/ImageUtils.h"
#include "../../utils/MemoryUtils.h"
#include "../../utils/RenderUtils.h"
#include "../../vulkan/DeviceManager.h"
#include "../../vulkan/SwapchainManager.h"
#include "../../utils/FileUtils.h"
#include "../../materials/ShaderObject.h"
#include "../../structs/PushConstantBlock.h"
#include "../../materials/Material.h"
#include "../../utils/DescriptorUtils.h"
#include "../../Config.h"
#include "../Vk_DynamicRendering.h"
#include "../../core/DrawableEntity.h"
#include "../../core/Engine.h"
#include "../../structs/Vertex_ObjectPicking.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/string_cast.hpp>
#include "../../utils/Initializers.h"

rendering::ObjectPicking::ObjectPicking(EngineContext& engine_context): engine_context(engine_context), command_pool(nullptr)
{
    device_manager = engine_context.device_manager.get();
    swapchain_manager = engine_context.swapchain_manager.get();
    first_submit_done = false;
}

void rendering::ObjectPicking::init_picking()
{
    utils::RenderUtils::create_command_pool(engine_context, command_pool,  vkb::QueueType::graphics);
    utils::RenderUtils::get_supported_depth_stencil_format(device_manager->get_physical_device(), &depth_stencil_image.format);
    utils::RenderUtils::create_depth_stencil_image(engine_context, swapchain_manager->get_swapchain().extent, device_manager->get_allocator(), depth_stencil_image);

    create_image_attachment();
    create_readback_id_buffer();
    create_normal_readback_buffer();

    create_object_picking_material();

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    auto dispatch_table = engine_context.dispatch_table;
    
    if (dispatch_table.allocateCommandBuffers(&allocInfo, &command_buffer) != VK_SUCCESS)
    {
        // failed to allocate command buffers;
        std::cerr << "failed to allocate command buffers for object picker\n";
        return;
    }

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = 0;
    dispatch_table.createFence(&fence_info, nullptr, &object_picker_fence);
}

void rendering::ObjectPicking::create_image_attachment()
{
    VkFormat image_format = VK_FORMAT_R32G32B32A32_SFLOAT;

    auto width = swapchain_manager->get_swapchain().extent.width;
    auto height = swapchain_manager->get_swapchain().extent.height;
    VkImageCreateInfo image_info = utils::ImageUtils::image_create_info(image_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, { width, height, 1 });

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    
    vmaCreateImage(device_manager->get_allocator(), &image_info, &alloc_info, &object_id_image.image, &object_id_image.allocation, nullptr);

    utils::ImageUtils::create_image_view(engine_context.dispatch_table, object_id_image, image_format);
}

void rendering::ObjectPicking::create_readback_id_buffer()
{
    auto swapchain_extents = engine_context.swapchain_manager->get_swapchain().extent;
    utils::MemoryUtils::allocate_buffer_with_readback_access(device_manager->get_allocator(), swapchain_extents.width * swapchain_extents.height * sizeof(glm::vec4), readback_id_buffer);
}

void rendering::ObjectPicking::create_normal_readback_buffer()
{
    utils::MemoryUtils::allocate_buffer_with_readback_access(device_manager->get_allocator(),  sizeof(glm::vec4), normal_readback_buffer);
    normal_readback_buffer_address = utils::MemoryUtils::get_buffer_device_address(engine_context.dispatch_table, normal_readback_buffer.buffer);
}

void rendering::ObjectPicking::create_object_picking_material()
{
    Vk_ShaderInfo shader_info_data =
    {
        .vertex_shader_path = std::string(SHADER_PATH) + "/object_picking_shader.vert.spv",
        .fragment_shader_path = std::string(SHADER_PATH) + "/object_picking_shader.frag.spv"
    };
    
    //Setup push constants
    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(ObjectPickerPushConstantBlock);

    size_t shaderCodeSizes[2]{};
    char* shaderCodes[2]{};

    utils::FileUtils::loadShader(shader_info_data.vertex_shader_path, shaderCodes[0], shaderCodeSizes[0]);
    utils::FileUtils::loadShader(shader_info_data.fragment_shader_path, shaderCodes[1], shaderCodeSizes[1]);

    auto shader_object = std::make_unique<material::ShaderObject>();
    shader_object->create_shaders(engine_context.dispatch_table,
        shaderCodes[0], shaderCodeSizes[0], shaderCodes[1], shaderCodeSizes[1], nullptr,  0, &push_constant_range, 1);

    VkPipelineLayout pipeline_layout;

    //Create the pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = initializers::pipelineLayoutCreateInfo(nullptr, 0, &push_constant_range, 1);
    engine_context.dispatch_table.createPipelineLayout(&pipelineLayoutInfo, VK_NULL_HANDLE, &pipeline_layout);

    //Create material 
    object_picker_material = std::make_unique<material::Material>("ObjectPicker");
    object_picker_material->add_shader_object(std::move(shader_object));
    object_picker_material->add_pipeline_layout(pipeline_layout);
    object_picker_material->add_descriptor_set(nullptr);
}

bool rendering::ObjectPicking::record_command_buffer(int32_t mouse_x, int32_t mouse_y)
{
    auto dispatch_table = engine_context.dispatch_table;

    if (first_submit_done)
    {
        dispatch_table.waitForFences(1, &object_picker_fence, VK_TRUE, UINT64_MAX);
        dispatch_table.resetFences(1, &object_picker_fence);
    }

    dispatch_table.resetCommandBuffer(command_buffer, 0);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (dispatch_table.beginCommandBuffer(command_buffer, &beginInfo) != VK_SUCCESS)
    {
        // failed to begin recording command buffer;
        return false;
    }

    Vk_DynamicRendering::image_layout_transition(command_buffer,
                                      object_id_image.image,
                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      0,
                                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                       VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    Vk_DynamicRendering::image_layout_transition(command_buffer,
                                  depth_stencil_image.image,
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                  0,
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                   VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });

    //Color attachment
    VkRenderingAttachmentInfoKHR color_attachment_info = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    color_attachment_info.pNext = VK_NULL_HANDLE;
    color_attachment_info.imageView                    = object_id_image.view;       
    color_attachment_info.imageLayout                  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_info.resolveMode                  = VK_RESOLVE_MODE_NONE;
    color_attachment_info.loadOp                       = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment_info.storeOp                      = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_info.clearValue                   = { { 1.0, 1.0, 0.0, 1.0 } };

    //Depth Stencil
    VkRenderingAttachmentInfoKHR depth_attachment_info = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    depth_attachment_info.pNext = VK_NULL_HANDLE;
    depth_attachment_info.imageView = depth_stencil_image.view;
    depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;
    depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment_info.clearValue = {{1.0f}};

    auto render_extent = swapchain_manager->get_swapchain().extent;
    auto render_area   = VkRect2D{VkOffset2D{0, 0}, render_extent};
    auto render_info   = Vk_DynamicRendering::rendering_info(render_area, 1, &color_attachment_info);

    render_info.layerCount       = 1;
    render_info.pColorAttachments = &color_attachment_info;
    render_info.pDepthAttachment = &depth_attachment_info;
    //render_info.pStencilAttachment = &depth_attachment_info;

    dispatch_table.cmdBeginRenderingKHR(command_buffer, &render_info);

    //Bind the shader
    
    object_picker_material->get_shader_object()->set_initial_state(dispatch_table,
                                                                   engine_context.swapchain_manager->get_swapchain().extent,
                                                                   command_buffer,
                                                                   Vertex_ObjectPicking::get_binding_description(), Vertex_ObjectPicking::get_attribute_descriptions(),
                                                                   engine_context.swapchain_manager->get_swapchain().extent,
                                                                   {0, 0});
    
    object_picker_material->get_shader_object()->bind_material_shader(engine_context.dispatch_table, command_buffer);

    ObjectPickerPushConstantBlock push_constants{};
    push_constants.scene_buffer_addr = engine_context.renderer->get_gpu_scene_buffer().scene_buffer_address;

    //Stores the currently selected face normal
    push_constants.face_normal_addr = normal_readback_buffer_address;
    
    //Draw the objects
    for (const auto& entity : core::Engine::get_drawable_entities())
    {
        RenderData render_data = entity->get_render_data();

        push_constants.model_transform_addr = entity->get_transform_buffer_address();
        push_constants.object_id_addr = entity->get_object_id_buffer_address();

        dispatch_table.cmdPushConstants(command_buffer, object_picker_material->get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ObjectPickerPushConstantBlock), &push_constants);

        VkBuffer vertex_buffers[] = { render_data.vertex_buffer.buffer };
        VkDeviceSize offsets[] = {0};
    
        dispatch_table.cmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
        dispatch_table.cmdBindIndexBuffer(command_buffer, render_data.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        dispatch_table.cmdDrawIndexed(command_buffer, render_data.indices.size(), 1, 0, 0, 0);
        
    }

    dispatch_table.cmdEndRenderingKHR(command_buffer);

    utils::ImageUtils::copy_image_to_buffer(engine_context, object_id_image, readback_id_buffer, command_buffer, { 0, 0, 0 });
    
    if (dispatch_table.endCommandBuffer(command_buffer) != VK_SUCCESS)
    {
        std::cout << "failed to record command buffer\n";
        return false; // failed to record command buffer!
    }
    
    return true;
}

glm::vec3 rendering::ObjectPicking::get_selected_face_normal(const glm::mat4& model_transform) const
{
    if(auto data = static_cast<glm::vec3*>(normal_readback_buffer.allocation_info.pMappedData))
    {
        glm::mat3 normal_matrix = inverse(transpose(glm::mat3(model_transform)));
        glm::vec3 world_normal = glm::normalize(normal_matrix * *data);
        
        return world_normal;

        // std::cout << "Data read from buffer: " << glm::to_string(*data) << std::endl;
        //
        // std::cout << "Model Transform (4x4): " << glm::to_string(model_transform) << std::endl;
        //
        // glm::mat3 model_transform_3x3 = glm::mat3(model_transform);
        // std::cout << "Model Transform (3x3): " << glm::to_string(model_transform_3x3) << std::endl;
        //
        // glm::mat3 transposed_matrix = glm::transpose(model_transform_3x3);
        // std::cout << "Transposed Matrix: " << glm::to_string(transposed_matrix) << std::endl;
        //
        // glm::mat3 normal_matrix = glm::inverse(transposed_matrix);
        // std::cout << "Normal Matrix: " << glm::to_string(normal_matrix) << std::endl;
        //
        // glm::vec3 world_normal_unnormalized = normal_matrix * *data;
        // std::cout << "World Normal (unnormalized): " << glm::to_string(world_normal_unnormalized) << std::endl;
        //
        // glm::vec3 world_normal = glm::normalize(world_normal_unnormalized);
        // std::cout << "Final World Normal: " << glm::to_string(world_normal) << std::endl;
        //
        // return world_normal;
    }

    return glm::vec3(0.0);
}
