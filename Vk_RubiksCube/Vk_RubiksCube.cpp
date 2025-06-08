
#include "Vk_RubiksCube.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <thread>

#include "Config.h"
#include "core/Engine.h"
#include "materials/ShaderObject.h"
#include "rendering/Vk_DynamicRendering.h"
#include "structs/PushConstantBlock.h"
#include "structs/Vk_SceneData.h"
#include "utils/ModelLoaderUtils.h"
#include "utils/DescriptorUtils.h"
#include "utils/Vk_Utils.h"
#include "utils/ImageUtils.h"
#include "utils/MemoryUtils.h"



// int create_command_buffers(Init& init, RenderData& data)
// {
//     data.command_buffers.resize(init.swapchain.image_count);
//
//     VkCommandBufferAllocateInfo allocInfo = {};
//     allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//     allocInfo.commandPool = data.command_pool;
//     allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//     allocInfo.commandBufferCount = static_cast<uint32_t>(data.command_buffers.size());
//
//     if (init.disp.allocateCommandBuffers(&allocInfo, data.command_buffers.data()) != VK_SUCCESS)
//     {
//         // failed to allocate command buffers;
//         return -1;
//     }
//
//     for (size_t i = 0; i < data.command_buffers.size(); i++)
//     {
//         VkCommandBufferBeginInfo begin_info = {};
//         begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//
//         if (init.disp.beginCommandBuffer(data.command_buffers[i], &begin_info) != VK_SUCCESS)
//         {
//             return -1;
//             // failed to begin recording command buffer
//         }
//
//         Vk_DynamicRendering::image_layout_transition(data.command_buffers[i],
//                                          init.swapchain.get_images().value()[i],
//                                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
//                                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//                                          0,
//                                          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
//                                          VK_IMAGE_LAYOUT_UNDEFINED,
//                                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//                                           VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
//
//         Vk_DynamicRendering::image_layout_transition(data.command_buffers[i],
//                                          data.depthStencilImage.image,
//                                          VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
//                                          VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
//                                          0,
//                                          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
//                                          VK_IMAGE_LAYOUT_UNDEFINED,
//                                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
//                                           VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });
//
//         //Color attachment
//         VkRenderingAttachmentInfoKHR color_attachment_info = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
//         color_attachment_info.pNext = VK_NULL_HANDLE;
//         color_attachment_info.imageView                    = init.swapchain.get_image_views().value()[i];        // color_attachment.image_view;
//         color_attachment_info.imageLayout                  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//         color_attachment_info.resolveMode                  = VK_RESOLVE_MODE_NONE;
//         color_attachment_info.loadOp                       = VK_ATTACHMENT_LOAD_OP_CLEAR;
//         color_attachment_info.storeOp                      = VK_ATTACHMENT_STORE_OP_STORE;
//         color_attachment_info.clearValue                   = {0.1f, 0.1f, 0.1f, 1.0f};
//
//         //Depth Stencil
//         VkRenderingAttachmentInfoKHR depth_attachment_info = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
//         depth_attachment_info.pNext = VK_NULL_HANDLE;
//         depth_attachment_info.imageView = data.depthStencilImage.view;
//         depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//         depth_attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;
//         depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//         depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//         depth_attachment_info.clearValue = {1.0f };
//
//         auto render_area             = VkRect2D{VkOffset2D{}, VkExtent2D{init.swapchain.extent.width, init.swapchain.extent.height}};
//         auto render_info             = Vk_DynamicRendering::rendering_info(render_area, 1, &color_attachment_info);
//
//         render_info.layerCount       = 1;
//         render_info.pColorAttachments = &color_attachment_info;
//         render_info.pDepthAttachment = &depth_attachment_info;
//         render_info.pStencilAttachment = &depth_attachment_info;
//         
//         init.disp.cmdBeginRenderingKHR(data.command_buffers[i], &render_info);
//         
//         data.shader_object->set_initial_state(init, data.command_buffers[i]);
//         
//         //Draw Stuff goes here
//         //Bind buffers
//         VkBuffer vertexBuffers[] = {data.vertexBuffer};
//         VkDeviceSize offsets[] = {0};
//         init.disp.cmdBindVertexBuffers(data.command_buffers[i], 0, 1, vertexBuffers, offsets);
//         init.disp.cmdBindIndexBuffer(data.command_buffers[i], data.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
//         init.disp.cmdBindDescriptorSets(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, init.pipelineLayout, 0, 1, &data.descriptorSet, 0, nullptr);
//         data.shader_object->bind_material_shader(init.disp, data.command_buffers[i]);
//
//         //Passing Buffer Addresses
//         PushConstantBlock references{};
//         // Pass pointer to the global matrix via a buffer device address
//         references.sceneBufferAddress = data.sceneData.sceneBufferAddress;
//         references.materialParamsAddress = data.materialValues.material_params_buffer_address;
//         
//         init.disp.cmdPushConstants(data.command_buffers[i], init.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantBlock), &references);
//         
//         // Issue the draw call using the index buffer
//         init.disp.cmdDrawIndexed(data.command_buffers[i], static_cast<uint32_t>(data.outIndices.size()), 1, 0, 0,0);
//         init.disp.cmdEndRenderingKHR(data.command_buffers[i]);
//
//         Vk_DynamicRendering::image_layout_transition
//         (
//             data.command_buffers[i],                            // Command buffer
//             init.swapchain.get_images().value()[i],               // Swapchain image
//             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // Source pipeline stage
//             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,     // Destination pipeline stage
//             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,     // Source access mask
//             0,                                        // Destination access mask
//             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // Old layout
//             VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,          // New layout
//              VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
//
//         if (init.disp.endCommandBuffer(data.command_buffers[i]) != VK_SUCCESS)
//         {
//             std::cout << "failed to record command buffer\n";
//             return -1; // failed to record command buffer!
//         }
//     }
//     return 0;
// }
//
// int create_sync_objects(Init& init, RenderData& data)
// {
//     data.available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
//     data.finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
//     data.in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
//     data.image_in_flight.resize(init.swapchain.image_count, VK_NULL_HANDLE);
//
//     VkSemaphoreCreateInfo semaphore_info = {};
//     semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//     VkFenceCreateInfo fence_info = {};
//     fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//     fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//     for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
//     {
//         if (init.disp.createSemaphore(&semaphore_info, nullptr, &data.available_semaphores[i]) != VK_SUCCESS ||
//             init.disp.createSemaphore(&semaphore_info, nullptr, &data.finished_semaphore[i]) != VK_SUCCESS ||
//             init.disp.createFence(&fence_info, nullptr, &data.in_flight_fences[i]) != VK_SUCCESS)
//         {
//             std::cout << "failed to create sync objects\n";
//             return -1; // failed to create synchronization objects for a frame
//         }
//     }
//     return 0;
// }
//
// int draw_frame(Init& init, RenderData& data)
// {
//     init.disp.waitForFences(1, &data.in_flight_fences[data.current_frame], VK_TRUE, UINT64_MAX);
//     uint32_t image_index = 0;
//     VkResult result = init.disp.acquireNextImageKHR(
//         init.swapchain, UINT64_MAX, data.available_semaphores[data.current_frame], VK_NULL_HANDLE, &image_index);
//     if (result == VK_ERROR_OUT_OF_DATE_KHR) 
//     {
//         return recreate_swapchain(init, data);
//     } 
//     else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
//     {
//         std::cout << "failed to acquire swapchain image. Error " << result << "\n";
//         return -1;
//     }
//
//     if (data.image_in_flight[image_index] != VK_NULL_HANDLE) 
//     {
//         init.disp.waitForFences(1, &data.image_in_flight[image_index], VK_TRUE, UINT64_MAX);
//     }
//     data.image_in_flight[image_index] = data.in_flight_fences[data.current_frame];
//
//     VkSubmitInfo submitInfo = {};
//     submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//     VkSemaphore wait_semaphores[] = { data.available_semaphores[data.current_frame] };
//     VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
//     submitInfo.waitSemaphoreCount = 1;
//     submitInfo.pWaitSemaphores = wait_semaphores;
//     submitInfo.pWaitDstStageMask = wait_stages;
//
//     submitInfo.commandBufferCount = 1;
//     submitInfo.pCommandBuffers = &data.command_buffers[image_index];
//
//     VkSemaphore signal_semaphores[] = { data.finished_semaphore[data.current_frame] };
//     submitInfo.signalSemaphoreCount = 1;
//     submitInfo.pSignalSemaphores = signal_semaphores;
//
//     init.disp.resetFences(1, &data.in_flight_fences[data.current_frame]);
//     if (init.disp.queueSubmit(data.graphics_queue, 1, &submitInfo, data.in_flight_fences[data.current_frame]) != VK_SUCCESS) 
//     {
//         std::cout << "failed to submit draw command buffer\n";
//         return -1; //"failed to submit draw command buffer
//     }
//
//     VkPresentInfoKHR present_info = {};
//     present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//
//     present_info.waitSemaphoreCount = 1;
//     present_info.pWaitSemaphores = signal_semaphores;
//
//     VkSwapchainKHR swapChains[] = { init.swapchain };
//     present_info.swapchainCount = 1;
//     present_info.pSwapchains = swapChains;
//
//     present_info.pImageIndices = &image_index;
//
//     result = init.disp.queuePresentKHR(data.present_queue, &present_info);
//     if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
//     {
//         return recreate_swapchain(init, data);
//     }
//     else if (result != VK_SUCCESS)
//     {
//         std::cout << "failed to present swapchain image\n";
//         return -1;
//     }
//
//     data.current_frame = (data.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
//     return 0;
// }
//
// void cleanup(Init& init, RenderData& data)
// {
//     for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
//     {
//         init.disp.destroySemaphore(data.finished_semaphore[i], nullptr);
//         init.disp.destroySemaphore(data.available_semaphores[i], nullptr);
//         init.disp.destroyFence(data.in_flight_fences[i], nullptr);
//     }
//
//     init.disp.destroyCommandPool(data.command_pool, nullptr);
//
//     init.disp.destroyPipeline(data.graphics_pipeline, nullptr);
//     init.disp.destroyPipelineLayout(data.pipeline_layout, nullptr);
//
//     init.swapchain.destroy_image_views(data.swapchain_image_views);
//
//     vkb::destroy_swapchain(init.swapchain);
//     vkb::destroy_device(init.device);
//     vkb::destroy_surface(init.instance, init.surface);
//     vkb::destroy_instance(init.instance);
//     destroy_window_glfw(init.window);
// }
//
// void loadModel(Init& init, RenderData& renderData)
// {
//     VkUtils::ModelLoaderUtils modelUtils;
//     modelUtils.load_obj(std::string(RESOURCE_PATH) + "/models/rubiks_cube/RubiksCubeTextures/rubiksCubeTexture.obj", renderData.outVertices, renderData.outIndices, renderData.primitiveMaterialIndices, renderData.material_params, renderData.textureInfo);
//
//     for (const auto& texture : renderData.textureInfo)
//     {
//         LoadedImageData imgData = VMA_ImageUtils::loadImageFromFile(texture.second.path);
//         VMA_ImageUtils::textures.push_back(VMA_ImageUtils::createAndUploadImage(init, renderData, imgData));
//     }
//     
//     std::cout << "Vertices: " << renderData.outVertices.size() << std::endl;
//     std::cout << "Indices: " << renderData.outIndices.size() << std::endl;
//
//
//     // data.outVertices = {
//     //     // Front face (Z = +0.5)
//     //     {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 0: Bottom-left-front
//     //     {{ 0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 1: Bottom-right-front
//     //     {{ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 2: Top-right-front
//     //     {{-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 3: Top-left-front
//     //
//     //     // Back face (Z = -0.5)
//     //     {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 4: Bottom-left-back
//     //     {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 5: Bottom-right-back
//     //     {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 6: Top-right-back
//     //     {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}  // 7: Top-left-back
//     // };
//     //
//     // // Define index data (36 indices for 12 triangles)
//     // // This defines the triangles using the vertices above.
//     // // Assuming uint32_t for indices, change to uint16_t if you use that.
//     // data.outIndices = {
//     //     // Front face
//     //     0, 1, 2, 0, 2, 3,
//     //     // Back face
//     //     4, 6, 5, 4, 7, 6,
//     //     // Top face
//     //     3, 2, 6, 3, 6, 7,
//     //     // Bottom face
//     //     0, 4, 5, 0, 5, 1,
//     //     // Right face
//     //     1, 5, 6, 1, 6, 2,
//     //     // Left face
//     //     4, 0, 3, 4, 3, 7
//     // };
// }

int main()
{
    core::Engine engine;
    engine.init();
    engine.run();
    engine.cleanup();
    
    // if (0 != device_initialization(init)) return -1;
    // if (0 != create_swapchain(init)) return -1;
    // if (0 != get_queues(init, render_data)) return -1;
    //
    // vmaUtils::createVmaAllocator(init);
    //
    // if (0 != create_command_pool(init, render_data)) return -1;
    //
    // //load model
    // loadModel(init, render_data);
    // vmaUtils::createVertexAndIndexBuffersVMA(init.vmaAllocator, init.disp, render_data.graphics_queue, render_data.command_pool, render_data, render_data.outVertices, render_data.outIndices);
    //
    // if (0 != create_graphics_pipeline(init, render_data)) return -1;
    // if (0 != create_command_buffers(init, render_data)) return -1;
    // if (0 != create_sync_objects(init, render_data)) return -1;
    //
    // while (!glfwWindowShouldClose(init.window))
    // {
    //     glfwPollEvents();
    //     int res = draw_frame(init, render_data);
    //     if (res != 0)
    //     {
    //         std::cout << "failed to draw frame \n";
    //         return -1;
    //     }
    // }
    //
    // init.disp.deviceWaitIdle();
    //
    // cleanup(init, render_data);
    return 0;
}
