#include "Renderer.h"

#include <iostream>

#include "../utils/DescriptorUtils.h"
#include "../utils/MemoryUtils.h"
#include "../utils/Vk_Utils.h"
#include "../vulkan/DeviceManager.h"
#include "../rendering/Vk_DynamicRendering.h"
#include "../structs/EngineContext.h"
#include "../structs/PushConstantBlock.h"
#include "../materials/ShaderObject.h"
#include "../vulkan/SwapchainManager.h"
#include "../structs/DrawItem.h"
#include "../materials/Material.h"
#include "../materials/MaterialManager.h"
#include "../utils/RenderUtils.h"
#include "picking/ObjectPicking.h"

core::Renderer::Renderer(EngineContext& engine_context) : scene_data(), gpu_scene_buffer(),
                                                          engine_context(engine_context),
                                                          command_pool(nullptr)
{
    device_manager = engine_context.device_manager.get();
    swapchain_manager = engine_context.swapchain_manager.get();
    dispatch_table = engine_context.dispatch_table;
    object_picker = std::make_unique<rendering::ObjectPicking>(engine_context);
}

void core::Renderer::init()
{
    create_sync_objects();
    setup_scene_data();
    
    utils::RenderUtils::create_command_pool(engine_context, command_pool);
    utils::RenderUtils::get_supported_depth_stencil_format(device_manager->get_physical_device(), &depth_stencil_image.format);
    utils::RenderUtils::create_depth_stencil_image(engine_context, swapchain_manager->get_swapchain().extent, device_manager->get_allocator(), depth_stencil_image);

    init_object_picker();
}

void core::Renderer::init_object_picker()
{
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    engine_context.dispatch_table.createSemaphore(&semaphore_info, nullptr, &object_picker_done_semaphore);
    object_picker->init_picking();
}

void core::Renderer::submit_object_picker_command_buffer() const
{
    auto buffer = object_picker->get_command_buffer();
    auto object_picker_fence = object_picker->get_object_picker_fence();
    
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &buffer;
    submit_info.pSignalSemaphores = &object_picker_done_semaphore;
    submit_info.signalSemaphoreCount = 1;
    dispatch_table.queueSubmit(device_manager->get_graphics_queue(), 1, &submit_info, object_picker_fence);

    object_picker->first_submit_done = true;
}

bool core::Renderer::draw_frame()
{
    dispatch_table.waitForFences(1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
    
    uint32_t image_index = 0;
    VkResult result = dispatch_table.acquireNextImageKHR(swapchain_manager->get_swapchain(), UINT64_MAX, available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        return swapchain_manager->recreate_swapchain(engine_context);
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        //std::cout << "failed to acquire swapchain image. Error " << result << "\n";
        return false;
    }

    if (image_in_flight[image_index] != VK_NULL_HANDLE) 
    {
        dispatch_table.waitForFences(1, &image_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }
    
    image_in_flight[image_index] = in_flight_fences[current_frame];
    
    //Object picker
    submit_object_picker_command_buffer();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = { available_semaphores[current_frame], object_picker_done_semaphore};
    VkPipelineStageFlags wait_stages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    
    submitInfo.waitSemaphoreCount = 2;
    submitInfo.pWaitSemaphores = wait_semaphores;
    submitInfo.pWaitDstStageMask = wait_stages;

    std::vector command_buffers_to_submit = { command_buffers[image_index] };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = command_buffers_to_submit.data();

    VkSemaphore signal_semaphores[] = { finished_semaphores[current_frame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signal_semaphores;

    dispatch_table.resetFences(1, &in_flight_fences[current_frame]);
    if (dispatch_table.queueSubmit(device_manager->get_graphics_queue(), 1, &submitInfo, in_flight_fences[current_frame]) != VK_SUCCESS) 
    {
        //std::cout << "failed to submit draw command buffer\n";
        return false; 
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapChains[] = { swapchain_manager->get_swapchain().swapchain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapChains;

    present_info.pImageIndices = &image_index;

    result = dispatch_table.queuePresentKHR(device_manager->get_present_queue(), &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        return swapchain_manager->recreate_swapchain(engine_context);
    }
    if (result != VK_SUCCESS)
    {
        //std::cout << "failed to present swapchain image\n";
        return false;
    }

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    return true;
}

bool core::Renderer::setup_scene_data()
{
    //Allocate buffer for scene data
    utils::MemoryUtils::allocate_buffer_with_mapped_access(device_manager->get_allocator(), sizeof(Vk_SceneData), gpu_scene_buffer.scene_buffer);

    //Get its address and other params
    gpu_scene_buffer.scene_buffer_address = utils::MemoryUtils::get_buffer_device_address(dispatch_table, gpu_scene_buffer.scene_buffer.buffer);

    //Fill and map the memory region
    utils::prepare_ubo(scene_data);
    utils::MemoryUtils::map_persistent_data(device_manager->get_allocator(), gpu_scene_buffer.scene_buffer.allocation, gpu_scene_buffer.scene_buffer.allocation_info, &scene_data, sizeof(Vk_SceneData));

    return true;
}

bool core::Renderer::create_sync_objects()
{
    available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    image_in_flight.resize(swapchain_manager->get_swapchain().image_count, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (dispatch_table.createSemaphore(&semaphore_info, nullptr, &available_semaphores[i]) != VK_SUCCESS ||
            dispatch_table.createSemaphore(&semaphore_info, nullptr, &finished_semaphores[i]) != VK_SUCCESS ||
            dispatch_table.createFence(&fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS)
        {
            std::cout << "failed to create sync objects\n";
            return false;
        }
    }

    return true;
}

bool core::Renderer::create_command_buffers()
{
    auto swapchain = engine_context.swapchain_manager->get_swapchain();
    command_buffers.resize(swapchain.image_count);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

    if (dispatch_table.allocateCommandBuffers(&allocInfo, command_buffers.data()) != VK_SUCCESS)
    {
        // failed to allocate command buffers;
        return false;
    }

    for (size_t i = 0; i < command_buffers.size(); i++)
    {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (dispatch_table.beginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS)
        {
            return false;
        }

        Vk_DynamicRendering::image_layout_transition(command_buffers[i],
                                      swapchain.get_images().value()[i],
                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      0,
                                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                       VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        Vk_DynamicRendering::image_layout_transition(command_buffers[i],
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
        color_attachment_info.imageView                    = swapchain.get_image_views().value()[i];       
        color_attachment_info.imageLayout                  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment_info.resolveMode                  = VK_RESOLVE_MODE_NONE;
        color_attachment_info.loadOp                       = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment_info.storeOp                      = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment_info.clearValue                   = {0.1f, 0.1f, 0.1f, 1.0f};

        //Depth Stencil
        VkRenderingAttachmentInfoKHR depth_attachment_info = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        depth_attachment_info.pNext = VK_NULL_HANDLE;
        depth_attachment_info.imageView = depth_stencil_image.view;
        depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth_attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;
        depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment_info.clearValue = {{1.0f}};

        auto render_area             = VkRect2D{VkOffset2D{}, VkExtent2D{swapchain.extent.width, swapchain.extent.height}};
        auto render_info             = Vk_DynamicRendering::rendering_info(render_area, 1, &color_attachment_info);

        render_info.layerCount       = 1;
        render_info.pColorAttachments = &color_attachment_info;
        render_info.pDepthAttachment = &depth_attachment_info;
        render_info.pStencilAttachment = &depth_attachment_info;

        dispatch_table.cmdBeginRenderingKHR(command_buffers[i], &render_info);
        
        for (const auto& [material_id, draw_batch] : draw_batches)
        {
            //Pipeline object binding
            draw_batch.material->get_shader_object()->set_initial_state(engine_context.dispatch_table, engine_context.swapchain_manager->get_swapchain().extent, command_buffers[i],
                                                                        Vertex::get_binding_description(), Vertex::get_attribute_descriptions(), engine_context.swapchain_manager->get_swapchain().extent); 
            draw_batch.material->get_shader_object()->bind_material_shader(engine_context.dispatch_table, command_buffers[i]);

            dispatch_table.cmdBindDescriptorSets(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, draw_batch.material->get_pipeline_layout(), 0, 1, &draw_batch.material->get_descriptor_set(), 0, nullptr);

            //Passing Buffer Addresses
            PushConstantBlock references{};
            // Pass a pointer to the global matrix via a buffer device address
            references.scene_buffer_address = engine_context.renderer->get_gpu_scene_buffer().scene_buffer_address;
            references.material_params_address = engine_context.material_manager->get_material_params_address();

            //Binds and draws meshes
            for (auto draw_item : draw_batch.items)
            {
                references.object_model_transform_addr = draw_item.entity->get_transform_buffer_address();

                dispatch_table.cmdPushConstants(command_buffers[i], draw_batch.material->get_pipeline_layout(),
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantBlock), &references);
                
                VkBuffer vertex_buffers[] = {draw_item.vertex_buffer};
                VkDeviceSize offsets[] = {0};
                dispatch_table.cmdBindVertexBuffers(command_buffers[i], 0, 1, vertex_buffers, offsets);
                dispatch_table.cmdBindIndexBuffer(command_buffers[i], draw_item.index_buffer, 0, VK_INDEX_TYPE_UINT32);
                
                // Issue the draw call using the index buffer
                dispatch_table.cmdDrawIndexed(command_buffers[i], draw_item.index_count, 1, draw_item.index_range.first, 0,0);
            }
        }
        
        dispatch_table.cmdEndRenderingKHR(command_buffers[i]);

        Vk_DynamicRendering::image_layout_transition
        (
             command_buffers[i],                            // Command buffer
             swapchain.get_images().value()[i],               // Swapchain image
             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // Source pipeline stage
             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,     // Destination pipeline stage
             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,     // Source access mask
             0,                                        // Destination access mask
             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // Old layout
             VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,          // New layout
              VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        if (dispatch_table.endCommandBuffer(command_buffers[i]) != VK_SUCCESS)
        {
            std::cout << "failed to record command buffer\n";
            return false;
        }
    }
    return true;
}
