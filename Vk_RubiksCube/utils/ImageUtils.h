#pragma once
#include <iostream>

#include "VkBootstrapDispatch.h"
#include "../structs/LoadedImageData.h"
#include "../structs/Vk_RenderData.h"
#include "../structs/Image.h"

struct Buffer;
struct Init;

namespace utils
{
    class ImageUtils
    {
        static LoadedImageData loadImageFromFile(const std::string& filePath, int desiredChannels = 4);

        static Image createAndUploadImage(const Init& init, const RenderData& renderData, const LoadedImageData& imageData);

        static VkImageCreateInfo imageCreateInfo(VkFormat imageFormat, VkImageUsageFlags imageUsageFlags, VkExtent3D imageExtent);

        static void copyImage(const Init& init, VkQueue queue, VkCommandPool command_pool, Buffer srcBuffer, Image dstImage, VkDeviceSize size, VkExtent3D extend, const LoadedImageData& imageData);

        static void createImageSampler(const Init& init, Image& image, VkFilter filter);

        static void createImageView(const Init& init, Image& image, VkFormat format);
    };
};
