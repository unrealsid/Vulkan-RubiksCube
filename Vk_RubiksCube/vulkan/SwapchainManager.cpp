#include "SwapchainManager.h"
#include <iostream>

#include "DeviceManager.h"
#include "../structs/EngineContext.h"
#include "../rendering/Renderer.h"
#include "../utils/RenderUtils.h"

bool vulkan::SwapchainManager::create_swapchain(const EngineContext& engine_context)
{
    vkb::SwapchainBuilder swapchain_builder{ engine_context.device_manager->get_device() };
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

bool vulkan::SwapchainManager::recreate_swapchain(const EngineContext& engine_context)
{
    auto device_manager = engine_context.device_manager.get();
    auto renderer = engine_context.renderer.get();
    engine_context.dispatch_table.deviceWaitIdle();

    engine_context.dispatch_table.destroyCommandPool(renderer->get_command_pool(), nullptr);

    swapchain.destroy_image_views(swapchain_image_views);
    if (create_swapchain(engine_context) == false)
        return false;
    //TODO: recreate command pool
    // if (renderer->create_command_pool() == false)
    //     return false;
    if (renderer->create_command_buffers() == false)
        return false;
    return true;
}