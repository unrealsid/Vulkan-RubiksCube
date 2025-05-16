#include "VMA_ImageUtils.h"
#include <stb_image.h>
#include <vk_mem_alloc.h>

#include "VkBootstrapDispatch.h"
#include "VMA_MemoryUtils.h"
#include "../structs/Image.h"
#include "../structs/Vk_Init.h"

LoadedImageData VMA_ImageUtils::loadImageFromFile(const std::string& filePath, int desiredChannels)
{
    LoadedImageData imageData;

    int tempOriginalChannels; // Used to store the original channels from the file
    
    imageData.pixels = stbi_load(filePath.c_str(), &imageData.width, &imageData.height, &tempOriginalChannels, desiredChannels);

    imageData.channels = desiredChannels; // Store the number of channels we requested
    imageData.originalChannels = tempOriginalChannels; // Store the original channels from the file

    if (!imageData.pixels)
    {
        std::cerr << "Failed to load image file: " << filePath << " - " << stbi_failure_reason() << std::endl;
        // Return an empty/invalid struct if loading failed
        imageData.width = 0;
        imageData.height = 0;
        imageData.channels = 0;
        imageData.originalChannels = 0;
    }
    else
    {
        std::cout << "Loaded image: " << filePath << " (" << imageData.width << "x" << imageData.height << ", " << imageData.channels << " channels)" << std::endl;
    }

    return imageData;
}

Image VMA_ImageUtils::createAndUploadImage(const Init& init, const RenderData& renderData, const LoadedImageData& imageData)
{
    if (imageData.pixels)
    {
        VkDeviceSize imageSize = imageData.width * imageData.height * imageData.channels;
        VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

        //allocate temporary buffer for holding texture data to upload
        //create staging buffer for image
        Buffer stagingImageBuffer;

        vmaUtils::createBufferVMA(init.vmaAllocator, imageSize,
                                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VMA_MEMORY_USAGE_AUTO,
                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |  VMA_ALLOCATION_CREATE_MAPPED_BIT,
                                  stagingImageBuffer.buffer, stagingImageBuffer.allocation, stagingImageBuffer.allocationInfo);
    
        void* data;
        vmaMapMemory(init.vmaAllocator, stagingImageBuffer.allocation, &data);
        memcpy(data, imageData.pixels, imageSize);
        vmaUnmapMemory(init.vmaAllocator, stagingImageBuffer.allocation);

        //Create image on the gpu
        VkExtent3D imageExtent;
        imageExtent.width = static_cast<uint32_t>(imageData.width);
        imageExtent.height = static_cast<uint32_t>(imageData.height);
        imageExtent.depth = 1;

        VkImageCreateInfo imgIfo = imageCreateInfo(imageFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

        //allocate and create the image
        VmaAllocationCreateInfo imgAllocInfo = {};
        imgAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        
        Image textureImage;
        vmaCreateImage(init.vmaAllocator, &imgIfo, &imgAllocInfo, &textureImage.image, &textureImage.allocation, &textureImage.allocationInfo);

        //TODO: Copy image to device Memory
        copyImage(init, renderData.graphics_queue, renderData.command_pool, stagingImageBuffer, textureImage, imageSize, imageExtent, imageData);

        createImageSampler(init, textureImage, VK_FILTER_LINEAR);

        createImageView(init, textureImage, imageFormat);
        
        return textureImage;
    }

    return {};
}

VkImageCreateInfo VMA_ImageUtils::imageCreateInfo(VkFormat imageFormat, VkImageUsageFlags imageUsageFlags, VkExtent3D imageExtent)
{
    VkImageCreateInfo imgInfo = {};
    imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.pNext = nullptr;
    imgInfo.imageType = VK_IMAGE_TYPE_2D;
    imgInfo.format = imageFormat;
    imgInfo.extent = imageExtent;
    imgInfo.mipLevels = 1;
    imgInfo.arrayLayers = 1;
    imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.usage = imageUsageFlags;
    imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imgInfo.queueFamilyIndexCount = 0;
    imgInfo.pQueueFamilyIndices = nullptr;
    imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgInfo.flags = 0;

    return imgInfo;
}

void VMA_ImageUtils::copyImage(const Init& init, VkQueue queue, VkCommandPool command_pool, Buffer srcBuffer, Image dstImage, VkDeviceSize size, VkExtent3D
                               extend, const LoadedImageData& imageData)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    init.disp.allocateCommandBuffers(&allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    init.disp.beginCommandBuffer(commandBuffer, &beginInfo);

    //Layout transition
    //1. convert image to Transfer dst. Image now ready to receive data
    VkImageSubresourceRange range;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    VkImageMemoryBarrier imageBarrier_toTransfer = {};
    imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier_toTransfer.image = dstImage.image;
    imageBarrier_toTransfer.subresourceRange = range;

    imageBarrier_toTransfer.srcAccessMask = 0;
    imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    //barrier the image into the transfer-receive layout
    //1.1 barrier
    init.disp.cmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;

    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageExtent = extend;

    //2. copy the buffer into the image
    init.disp.cmdCopyBufferToImage(commandBuffer, srcBuffer.buffer, dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    //3. Put barrier for image after copy
    VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

    imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    //barrier the image into the shader readable layout
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
    
    init.disp.endCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    init.disp.queueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    init.disp.queueWaitIdle(queue);

    init.disp.freeCommandBuffers(command_pool, 1, &commandBuffer);

    vmaDestroyBuffer(init.vmaAllocator, srcBuffer.buffer, srcBuffer.allocation);
}

void VMA_ImageUtils::createImageSampler(const Init& init, Image& image, VkFilter filter)
{
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = filter;
    samplerCreateInfo.minFilter = filter;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    init.disp.createSampler(&samplerCreateInfo, nullptr, &image.sampler);
}

void VMA_ImageUtils::createImageView(const Init& init, Image& image, VkFormat format)
{
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.pNext = NULL;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.image = image.image;
    init.disp.createImageView(&viewCreateInfo, nullptr, &image.view);
}
