#pragma once
#include <iostream>

#include "VkBootstrapDispatch.h"
#include "../structs/LoadedImageData.h"
#include "../structs/Vk_RenderData.h"
#include "../structs/Image.h"

struct Buffer;
struct Init;

namespace VMA_ImageUtils
{
    inline std::vector<Image> textures;
    
    LoadedImageData loadImageFromFile(const std::string& filePath, int desiredChannels = 4);

    Image createAndUploadImage(const Init& init, const RenderData& renderData, const LoadedImageData& imageData);

    VkImageCreateInfo imageCreateInfo(VkFormat imageFormat, VkImageUsageFlags imageUsageFlags, VkExtent3D imageExtent);

    void copyImage(const Init& init, VkQueue queue, VkCommandPool command_pool, Buffer srcBuffer, Image dstImage, VkDeviceSize size, VkExtent3D extend, const LoadedImageData& imageData);

    void createImageSampler(const Init& init, Image& image, VkFilter filter);

    void createImageView(const Init& init, Image& image, VkFormat format);
};
