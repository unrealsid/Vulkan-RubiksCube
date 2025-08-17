#include "SwapchainManager.h"
#include <iostream>

#include "DeviceManager.h"
#include "../structs/EngineContext.h"
#include "../rendering/Renderer.h"
#include "../utils/RenderUtils.h"

vulkan::SwapchainManager::~SwapchainManager()
{
    cleanup();
}

bool vulkan::SwapchainManager::create_swapchain(const EngineContext& engine_context)
{
    vkb::SwapchainBuilder swapchain_builder{ engine_context.device_manager->get_device() };
    
    // Set the old swapchain for proper recreation
    auto swap_ret = swapchain_builder
        .set_old_swapchain(swapchain)
        .set_desired_format({ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR) // Guaranteed to be available
        .add_fallback_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
        .add_fallback_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
        .build();
        
    if (!swap_ret)
    {
        std::cout << "Failed to create swapchain: " << swap_ret.error().message() << " " << swap_ret.vk_result() << "\n";
        return false;
    }

    // Clean up old swapchain if it exists
    if (swapchain.swapchain != VK_NULL_HANDLE)
    {
        cleanup_image_views(engine_context);
        vkb::destroy_swapchain(swapchain);
    }
    
    swapchain = swap_ret.value();
    
    // Get swapchain images
    auto images = swapchain.get_images();
    if (!images)
    {
        std::cout << "Failed to get swapchain images: " << images.error().message() << "\n";
        return false;
    }
    swapchain_images = images.value();
    
    return true;
}

bool vulkan::SwapchainManager::recreate_swapchain(const EngineContext& engine_context)
{
    auto device_manager = engine_context.device_manager.get();
    auto renderer = engine_context.renderer.get();
    
    // Wait for device to be idle
    engine_context.dispatch_table.deviceWaitIdle();

    // Clean up resources that depend on the swapchain
    if (renderer)
    {
        renderer->destroy_command_pool();
    }

    // Recreate the swapchain
    if (!create_swapchain(engine_context))
    {
        std::cout << "Failed to recreate swapchain\n";
        return false;
    }

    // Recreate dependent resources
    if (renderer)
    {
        // Recreate command pool
        if (!renderer->create_command_pool())
        {
            std::cout << "Failed to recreate command pool\n";
            return false;
        }
        
        // Recreate command buffers
        if(!renderer->recreate_depth_stencil_image())
        {
            std::cout << "Failed to recreate depth & stencil images\n";
            return false;
        }
        
        if (!renderer->create_command_buffers())
        {
            std::cout << "Failed to recreate command buffers\n";
            return false;
        }
    }

    return true;
}

void vulkan::SwapchainManager::cleanup_image_views(const EngineContext& engine_context)
{
    for (auto& image_view : swapchain_image_views)
    {
        if (image_view != VK_NULL_HANDLE)
        {
            engine_context.dispatch_table.destroyImageView(image_view, nullptr);
        }
    }
    swapchain_image_views.clear();
}

void vulkan::SwapchainManager::cleanup()
{
    if (swapchain.swapchain != VK_NULL_HANDLE)
    {
        vkb::destroy_swapchain(swapchain);
        swapchain = {};
    }
    swapchain_images.clear();
    swapchain_image_views.clear();
}