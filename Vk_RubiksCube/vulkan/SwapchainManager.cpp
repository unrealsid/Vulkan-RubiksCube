#include "SwapchainManager.h"
#include <iostream>

#include "DeviceManager.h"

bool vulkan::SwapchainManager::create_swapchain(const vulkan::DeviceManager& deviceManager)
{
    vkb::SwapchainBuilder swapchain_builder{ deviceManager.get_device() };
    auto swap_ret = swapchain_builder.set_old_swapchain(swapchain).build();
    if (!swap_ret)
    {
        std::cout << swap_ret.error().message() << " " << swap_ret.vk_result() << "\n";
        return false;
    }
    vkb::destroy_swapchain(swapchain);
    swapchain = swap_ret.value();
    return true;
}

bool vulkan::SwapchainManager::recreate_swapchain(vulkan::DeviceManager& deviceManager)
{
    deviceManager.get_dispatch_table().deviceWaitIdle();

    deviceManager.get_dispatch_table().destroyCommandPool(deviceManager.get_command_pool(), nullptr);

    swapchain.destroy_image_views(swapchain_image_views);
    if (create_swapchain(deviceManager) == false) return false;
    if (deviceManager.create_command_pool() == false) return false;
    if (deviceManager.create_command_buffers() == false) return false;
    return true;
}