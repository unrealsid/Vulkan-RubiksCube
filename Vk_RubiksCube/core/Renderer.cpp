#include "Renderer.h"

#include <iostream>

#include "../utils/DescriptorUtils.h"
#include "../utils/MemoryUtils.h"
#include "../utils/Vk_Utils.h"
#include "../vulkan/DeviceManager.h"

core::Renderer::Renderer(vulkan::DeviceManager* device_manager, vulkan::SwapchainManager* swapchain_manager)
{
    this->device_manager = device_manager;
    this->swapchain_manager = swapchain_manager;
    dispatch_table = this->device_manager->get_dispatch_table();
}

void core::Renderer::init()
{
    create_sync_objects();
    setup_scene_data();
    
    get_supported_depth_stencil_format(device_manager->get_physical_device(), &depth_stencil_image.format);
    create_depth_stencil_image(swapchain_manager->get_swapchain().extent, device_manager->get_allocator(), depth_stencil_image);
}

VkBool32 core::Renderer::get_supported_depth_stencil_format(VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat)
{
    std::vector<VkFormat> formatList =
    {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
    };

    for (auto& format : formatList)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            *depthStencilFormat = format;
            return true;
        }
    }

    return false;
}

void core::Renderer::create_depth_stencil_image(VkExtent2D extents, VmaAllocator allocator, DepthStencilImage& depthImage)
{
    VkImageCreateInfo imageCI{};
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = depthImage.format;
    imageCI.extent = { extents.width, extents.height, 1 };
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(allocator, &imageCI, &allocInfo, &depthImage.image, &depthImage.allocation, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create depth stencil image!");
    }

    VkImageViewCreateInfo imageViewCI{};
    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.image = depthImage.image;
    imageViewCI.format = depthImage.format;
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = 1;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = 1;
    imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    
    // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
    if (depthImage.format >= VK_FORMAT_D16_UNORM_S8_UINT)
    {
        imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    if (dispatch_table.createImageView(&imageViewCI, nullptr,  &depthImage.view) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create depth stencil image view!");
    }
}

bool core::Renderer::draw_frame()
{
    dispatch_table.waitForFences(1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
    uint32_t image_index = 0;
    VkResult result = dispatch_table.acquireNextImageKHR(
        swapchain_manager->get_swapchain(), UINT64_MAX, available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        return swapchain_manager->recreate_swapchain(*device_manager);
    } 
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        std::cout << "failed to acquire swapchain image. Error " << result << "\n";
        return false;
    }

    if (image_in_flight[image_index] != VK_NULL_HANDLE) 
    {
        dispatch_table.waitForFences(1, &image_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }
    image_in_flight[image_index] = in_flight_fences[current_frame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = { available_semaphores[current_frame] };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = wait_semaphores;
    submitInfo.pWaitDstStageMask = wait_stages;

    submitInfo.commandBufferCount = 1;
    const auto& command_buffers = device_manager->get_command_buffers();
    submitInfo.pCommandBuffers = &command_buffers[image_index];

    VkSemaphore signal_semaphores[] = { finished_semaphores[current_frame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signal_semaphores;

    dispatch_table.resetFences(1, &in_flight_fences[current_frame]);
    if (dispatch_table.queueSubmit(device_manager->get_graphics_queue(), 1, &submitInfo, in_flight_fences[current_frame]) != VK_SUCCESS) 
    {
        std::cout << "failed to submit draw command buffer\n";
        return -1; //"failed to submit draw command buffer
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
        return swapchain_manager->recreate_swapchain(*device_manager);
    }
    else if (result != VK_SUCCESS)
    {
        std::cout << "failed to present swapchain image\n";
        return false;
    }

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    return true;
}

bool core::Renderer::setup_scene_data()
{
    //Allocate buffer for scene data
    utils::MemoryUtils::allocate_buffer_with_mapped_access(device_manager->get_allocator(), sizeof(Vk_SceneData), gpu_scene_data.scene_buffer);

    //Get it's address and other params
    gpu_scene_data.scene_buffer_address = utils::MemoryUtils::getBufferDeviceAddress(device_manager->get_dispatch_table(), gpu_scene_data.scene_buffer.buffer);

    //Fill and map the memory region
    utils::prepare_ubo(scene_data);
    utils::MemoryUtils::mapPersistenData(device_manager->get_allocator(), gpu_scene_data.scene_buffer.allocation, gpu_scene_data.scene_buffer.allocation_info, &scene_data, sizeof(Vk_SceneData));

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