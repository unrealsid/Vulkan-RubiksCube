#include "SwapchainManager.h"
#include <iostream>

#include "DeviceManager.h"

bool vulkan::SwapchainManager::createSwapchain(const vulkan::DeviceManager& deviceManager)
{
    vkb::SwapchainBuilder swapchain_builder{ deviceManager.getDevice() };
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

bool vulkan::SwapchainManager::recreateSwapchain(vulkan::DeviceManager& deviceManager)
{
    deviceManager.getDispatchTable().deviceWaitIdle();

    deviceManager.getDispatchTable().destroyCommandPool(deviceManager.getCommandPool(), nullptr);

    swapchain.destroy_image_views(swapchain_image_views);
    if (createSwapchain(deviceManager) == false) return false;
    if (deviceManager.createCommandPool() == false) return false;
    if (deviceManager.createCommandBuffers() == false) return false;
    return true;
}