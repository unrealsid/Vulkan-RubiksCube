#pragma once
#include <vector>
#include <vulkan.h>

#include "VkBootstrap.h"

namespace vulkan
{
    class DeviceManager;
}

namespace vulkan
{
    class SwapchainManager
    {
    public:
        bool createSwapchain( const DeviceManager& deviceManager);
        bool recreateSwapchain( DeviceManager& deviceManager);

        [[nodiscard]] vkb::Swapchain getSwapchain() const { return swapchain; }

    private:    
        vkb::Swapchain swapchain;
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageView> swapchain_image_views;
    };
}
