#pragma once
#include <iostream>

#include "VkBootstrapDispatch.h"
#include "../structs/LoadedImageData.h"
#include "../structs/Vk_RenderData.h"
#include "../structs/Vk_Image.h"

struct EngineContext;

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

        static Vk_Image create_texture_image(EngineContext& engine_context,  const LoadedImageData& image_data);

        static VkImageCreateInfo image_create_info(VkFormat imageFormat, VkImageUsageFlags imageUsageFlags, VkExtent3D imageExtent);

        static void copy_image(EngineContext& engine_context, VkQueue queue, VkCommandPool command_pool, GPU_Buffer srcBuffer, Vk_Image dstImage, VkDeviceSize size, VkExtent3D extend, const LoadedImageData& imageData);

        static void copy_image_to_buffer(EngineContext& engine_context, Vk_Image src_image, GPU_Buffer& dst_buffer, VkCommandBuffer cmd_buffer, VkOffset3D
                                         image_offset);

        static void create_image_sampler(const vkb::DispatchTable& disp, Vk_Image& image, VkFilter filter);

        static void create_image_view(const vkb::DispatchTable& disp, Vk_Image& image, VkFormat format);
    };
};
