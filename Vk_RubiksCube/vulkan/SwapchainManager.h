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
        bool create_swapchain( const DeviceManager& deviceManager);
        bool recreate_swapchain( DeviceManager& deviceManager);

        [[nodiscard]] vkb::Swapchain get_swapchain() const { return swapchain; }

    private:    
        vkb::Swapchain swapchain;
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageView> swapchain_image_views;
    };
}
