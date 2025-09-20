#include "ImageUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vma/vk_mem_alloc.h>

#include "DescriptorUtils.h"
#include "../rendering/Vk_DynamicRendering.h"
#include "VkBootstrapDispatch.h"
#include "MemoryUtils.h"
#include "../structs/EngineContext.h"
#include "../structs/Vk_Image.h"
#include "../rendering/Renderer.h"
#include "../vulkan/DeviceManager.h"

LoadedImageData utils::ImageUtils::load_image_data(const std::string& filePath, int desiredChannels)
{
    LoadedImageData imageData;

    int tempOriginalChannels; // Used to store the original channels from the file
    
    imageData.pixels = stbi_load(filePath.c_str(), &imageData.width, &imageData.height, &tempOriginalChannels, desiredChannels);

    imageData.channels = desiredChannels; // Store the number of channels we requested
    imageData.original_channels = tempOriginalChannels; // Store the original channels from the file

    if (!imageData.pixels)
    {
        std::cerr << "Failed to load image file: " << filePath << " - " << stbi_failure_reason() << std::endl;
        // Return an empty/invalid struct if loading failed
        imageData.width = 0;
        imageData.height = 0;
        imageData.channels = 0;
        imageData.original_channels = 0;
    }
    else
    {
        std::cout << "Loaded image: " << filePath << " (" << imageData.width << "x" << imageData.height << ", " << imageData.channels << " channels)" << std::endl;
    }

    return imageData;
}

Vk_Image utils::ImageUtils::create_texture_image(EngineContext& engine_context, const LoadedImageData& image_data)
{
    if (image_data.pixels)
    {
        VkDeviceSize imageSize = image_data.width * image_data.height * image_data.channels;
        VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

        auto device_manager = engine_context.device_manager.get();
        auto renderer = engine_context.renderer.get();

        //allocate temporary buffer for holding texture data to upload
        //create staging buffer for image
        GPU_Buffer stagingImageBuffer;

        MemoryUtils::create_buffer(engine_context.dispatch_table,
                                     device_manager->get_allocator(), imageSize,
                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VMA_MEMORY_USAGE_AUTO,
                                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |  VMA_ALLOCATION_CREATE_MAPPED_BIT,
                                     stagingImageBuffer);
    
        void* data;
        vmaMapMemory(device_manager->get_allocator(), stagingImageBuffer.allocation, &data);
        memcpy(data, image_data.pixels, imageSize);
        vmaUnmapMemory(device_manager->get_allocator(), stagingImageBuffer.allocation);

        //Create image on the gpu
        VkExtent3D imageExtent;
        imageExtent.width = static_cast<uint32_t>(image_data.width);
        imageExtent.height = static_cast<uint32_t>(image_data.height);
        imageExtent.depth = 1;

        VkImageCreateInfo imgIfo = image_create_info(imageFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

        //allocate and create the image
        VmaAllocationCreateInfo imgAllocInfo = {};
        imgAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        
        Vk_Image textureImage;
        vmaCreateImage(device_manager->get_allocator(), &imgIfo, &imgAllocInfo, &textureImage.image, &textureImage.allocation, &textureImage.allocation_info);

        //Copy image to device Memory
        copy_image(engine_context, device_manager->get_graphics_queue(), renderer->get_command_pool(), stagingImageBuffer, textureImage, imageSize, imageExtent, image_data);

        create_image_sampler(engine_context.dispatch_table, textureImage, VK_FILTER_LINEAR);

        create_image_view(engine_context.dispatch_table, textureImage, imageFormat);
        
        return textureImage;
    }

    return {};
}

VkImageCreateInfo utils::ImageUtils::image_create_info(VkFormat imageFormat, VkImageUsageFlags imageUsageFlags, VkExtent3D imageExtent)
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

void utils::ImageUtils::copy_image(EngineContext& engine_context, VkQueue queue, VkCommandPool command_pool, GPU_Buffer srcBuffer, Vk_Image
                                  dstImage, VkDeviceSize size, VkExtent3D
                                  extend, const LoadedImageData& imageData)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    engine_context.dispatch_table.allocateCommandBuffers(&allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    engine_context.dispatch_table.beginCommandBuffer(commandBuffer, &beginInfo);

    //Layout transition
    //1. convert image to Transfer dst. Vk_Image now ready to receive data
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
    engine_context.dispatch_table.cmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

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
    engine_context.dispatch_table.cmdCopyBufferToImage(commandBuffer, srcBuffer.buffer, dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    //3. Put barrier for image after copy
    VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

    imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    //barrier the image into the shader readable layout
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
    
    engine_context.dispatch_table.endCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    engine_context.dispatch_table.queueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    engine_context.dispatch_table.queueWaitIdle(queue);

    engine_context.dispatch_table.freeCommandBuffers(command_pool, 1, &commandBuffer);

    vmaDestroyBuffer(engine_context.device_manager->get_allocator(), srcBuffer.buffer, srcBuffer.allocation);
}

void utils::ImageUtils::copy_image_to_buffer(EngineContext& engine_context, Vk_Image src_image, GPU_Buffer& dst_buffer, VkCommandBuffer cmd_buffer, VkOffset3D image_offset)
{
    auto dispatch_table = engine_context.dispatch_table;
    
    // Transition image for transfer
        Vk_DynamicRendering::image_layout_transition(
        cmd_buffer,
        src_image.image,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
    );
    //Copy the image to the buffer
    VkBufferImageCopy copy_region{};
    copy_region.bufferOffset = 0;
    copy_region.bufferRowLength = 0;
    copy_region.bufferImageHeight = 0;
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageOffset = image_offset;
    auto extents = engine_context.swapchain_manager->get_swapchain_extent();
    copy_region.imageExtent = {extents.width, extents.height, 1};
    
    dispatch_table.cmdCopyImageToBuffer(
        cmd_buffer,
        src_image.image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dst_buffer.buffer,
        1,
        &copy_region
    );

    //Add a barrier for the image 
    Vk_DynamicRendering::image_layout_transition(
        cmd_buffer,
        src_image.image,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_TRANSFER_READ_BIT,
        0,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
    );

    //And add a buffer for the buffer before it is read on the cpu
    VkBufferMemoryBarrier buffer_barrier{};
    buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    buffer_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    buffer_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_barrier.buffer = dst_buffer.buffer;
    buffer_barrier.offset = 0;
    buffer_barrier.size = VK_WHOLE_SIZE;

    engine_context.dispatch_table.cmdPipelineBarrier(
        cmd_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_HOST_BIT,
        0,
        0, nullptr,
        1, &buffer_barrier,
        0, nullptr
    );
}

void utils::ImageUtils::create_image_sampler(const vkb::DispatchTable& disp, Vk_Image& image, VkFilter filter)
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
    disp.createSampler(&samplerCreateInfo, nullptr, &image.sampler);
}

void utils::ImageUtils::create_image_view(const vkb::DispatchTable& disp, Vk_Image& image, VkFormat format)
{
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.pNext = nullptr;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.image = image.image;
    disp.createImageView(&viewCreateInfo, nullptr, &image.view);
}
