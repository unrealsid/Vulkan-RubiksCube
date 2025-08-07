#pragma once
#include <vector>
#include <vulkan.h>

#include "VkBootstrap.h"

struct EngineContext;

namespace vulkan
{
    class DeviceManager;
}

namespace vulkan
{
    class SwapchainManager
    {
    public:
        bool create_swapchain(const EngineContext& engine_context);
        bool recreate_swapchain(const EngineContext& engine_context);

        [[nodiscard]] vkb::Swapchain get_swapchain() const { return swapchain; }

    private:    
        vkb::Swapchain swapchain;
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageView> swapchain_image_views;
    };
}
