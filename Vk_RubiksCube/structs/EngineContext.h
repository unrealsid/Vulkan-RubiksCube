#pragma once
#include <memory>
#include "VkBootstrapDispatch.h"

namespace material
{
    class MaterialManager;
}

namespace core
{
    class Renderer;
}

namespace vulkan
{
    class SwapchainManager;
    class DeviceManager;
}

namespace window
{
    class WindowManager;
}

struct EngineContext
{
    std::unique_ptr<window::WindowManager> window_manager;
    std::unique_ptr<vulkan::DeviceManager> device_manager;
    std::unique_ptr<material::MaterialManager> material_manager;
    std::unique_ptr<core::Renderer> renderer;
    std::unique_ptr<vulkan::SwapchainManager> swapchain_manager;

    vkb::InstanceDispatchTable instance_dispatch_table;
    vkb::DispatchTable dispatch_table;
};
