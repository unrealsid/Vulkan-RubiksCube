#pragma once
#include <iostream>

#include "VkBootstrapDispatch.h"
#include "../structs/LoadedImageData.h"
#include "../structs/Vk_RenderData.h"
#include "../structs/Vk_Image.h"

namespace vulkan
{
    class DeviceManager;
}

struct GPU_Buffer;
struct Init;

namespace utils
{
    class ImageUtils
    {
    public:    
        static LoadedImageData load_image_data(const std::string& filePath, int desiredChannels = 4);

        static Vk_Image create_texture_image(const vulkan::DeviceManager& device_manager,  const LoadedImageData& imageData);

        static VkImageCreateInfo imageCreateInfo(VkFormat imageFormat, VkImageUsageFlags imageUsageFlags, VkExtent3D imageExtent);

        static void copyImage(const vulkan::DeviceManager& device_manager, VkQueue queue, VkCommandPool command_pool, GPU_Buffer srcBuffer, Vk_Image dstImage, VkDeviceSize size, VkExtent3D extend, const LoadedImageData& imageData);

        static void createImageSampler(const vkb::DispatchTable& disp, Vk_Image& image, VkFilter filter);

        static void createImageView(const vkb::DispatchTable& disp, Vk_Image& image, VkFormat format);
    };
};
