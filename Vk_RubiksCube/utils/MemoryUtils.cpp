#include "MemoryUtils.h"
#include <iostream>
#include "../structs/Vk_RenderData.h"
#include "../structs/Vertex.h"
#include "../structs/Vk_Init.h"

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "Vk_Descriptors.h"
#include "Vk_Utils.h"
#include "../vulkan/DeviceManager.h"


void utils::MemoryUtils::createVmaAllocator(vulkan::DeviceManager& device_manager)
{
    PFN_vkGetInstanceProcAddr fnGetInstanceProcAddr = (vkGetInstanceProcAddr);
    PFN_vkGetDeviceProcAddr fnGetDeviceProcAddr = (vkGetDeviceProcAddr);
    
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = fnGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = fnGetDeviceProcAddr;

    //TODO: Change later Needed to run with NVIDIA NSights
    vulkanFunctions.vkGetBufferMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2KHR>(vkGetInstanceProcAddr(device_manager.getInstance(), "vkGetBufferMemoryRequirements2KHR"));
    vulkanFunctions.vkGetImageMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetImageMemoryRequirements2KHR>(vkGetInstanceProcAddr(device_manager.getInstance(), "vkGetImageMemoryRequirements2KHR"));
    vulkanFunctions.vkGetBufferMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2KHR>(vkGetInstanceProcAddr(device_manager.getInstance(), "vkGetBufferMemoryRequirements2KHR"));
    vulkanFunctions.vkBindBufferMemory2KHR = reinterpret_cast<PFN_vkBindBufferMemory2KHR>(vkGetInstanceProcAddr(device_manager.getInstance(), "vkBindBufferMemory2KHR"));
    vulkanFunctions.vkBindImageMemory2KHR = reinterpret_cast<PFN_vkBindImageMemory2KHR>(vkGetInstanceProcAddr(device_manager.getInstance(), "vkBindImageMemory2KHR"));
    
    VmaAllocatorCreateInfo allocatorInfo;

    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorInfo.physicalDevice = device_manager.getDevice().physical_device;
    allocatorInfo.instance = device_manager.getInstance();
    allocatorInfo.device = device_manager.getDevice();
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;
    
    VkResult result = vmaCreateAllocator(&allocatorInfo, &device_manager.getAllocator());
    if (result != VK_SUCCESS)
    {
        std::cerr << "Failed to create VMA allocator: " << result << std::endl;
        throw std::runtime_error("Failed to create VMA allocator!");
    }

    std::cout << "VMA allocator created successfully." << std::endl;
}

void utils::MemoryUtils::createBufferVMA(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
                               VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags vmaAllocationFlags, VkBuffer& buffer, VmaAllocation& allocation, VmaAllocationInfo& allocationInfo)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = memoryUsage;  
    allocCreateInfo.flags = vmaAllocationFlags;

    if (VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocationInfo); result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer with VMA!");
    }
}

VkResult utils::MemoryUtils::mapPersistenData(VmaAllocator vmaAllocator, VmaAllocation allocation, const VmaAllocationInfo& allocationInfo, const void* data, VkDeviceSize bufferSize)
{
    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(vmaAllocator, allocation, &memPropFlags);
    
    if(memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        memcpy(allocationInfo.pMappedData, data, bufferSize);
        VkResult result = vmaFlushAllocation(vmaAllocator, allocation, 0, VK_WHOLE_SIZE);
        return result;
    }
    
     return VK_ERROR_UNKNOWN;
}

VkDeviceAddress utils::MemoryUtils::getBufferDeviceAddress(const vkb::DispatchTable& disp, VkBuffer buffer)
{
    VkBufferDeviceAddressInfoEXT bufferDeviceAI{};
    bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAI.buffer = buffer;

    PFN_vkGetDeviceProcAddr fnGetDeviceProcAddr = (vkGetDeviceProcAddr);
    PFN_vkGetBufferDeviceAddressEXT fnGetBufferDeviceAddressEXT = reinterpret_cast<PFN_vkGetBufferDeviceAddressEXT>(vkGetDeviceProcAddr(disp.device, "vkGetBufferDeviceAddressKHR"));
    
    return fnGetBufferDeviceAddressEXT(disp.device, &bufferDeviceAI);
}


void utils::MemoryUtils::copyBuffer(vkb::DispatchTable disp, VkQueue queue, VkCommandPool command_pool, VkBuffer srcBuffer,
                          VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    disp.allocateCommandBuffers(&allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    disp.beginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    disp.cmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    disp.endCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    disp.queueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    disp.queueWaitIdle(queue);

    disp.freeCommandBuffers(command_pool, 1, &commandBuffer);
}

void utils::MemoryUtils::createVertexAndIndexBuffersVMA(VmaAllocator vmaAllocator, vkb::DispatchTable disp, VkQueue queue, VkCommandPool command_pool, RenderData& renderData, const
                                              std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
    VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();

    // Create Staging Buffer for Vertices using VMA
    VkBuffer stagingVertexBuffer;
    VmaAllocation stagingVertexBufferAllocation;
    VmaAllocationInfo allocationInfo;
    createBufferVMA(vmaAllocator, vertexBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VMA_MEMORY_USAGE_AUTO,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |  VMA_ALLOCATION_CREATE_MAPPED_BIT,
                    stagingVertexBuffer, stagingVertexBufferAllocation, allocationInfo);

    // Copy Vertex Data to Staging Buffer using VMA mapping
    assert(vertices.size() != 0, "Vertex Data is empty!");
    
    void* data;
    vmaMapMemory(vmaAllocator, stagingVertexBufferAllocation, &data);
    memcpy(data, vertices.data(), (size_t) vertexBufferSize);
    vmaUnmapMemory(vmaAllocator, stagingVertexBufferAllocation);

    // Create Vertex Buffer (Device Local) using VMA
    createBufferVMA(vmaAllocator, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,renderData.vertexBuffer, renderData.vertexBufferAllocation, allocationInfo);

    // Copy from Staging Vertex Buffer to Device Local Vertex Buffer
    copyBuffer(disp, queue, command_pool, stagingVertexBuffer, renderData.vertexBuffer, vertexBufferSize);
    vkUtils::SetVulkanObjectName(disp, (uint64_t) renderData.vertexBuffer, VK_OBJECT_TYPE_BUFFER, "Vertex Buffer");

    // Clean up Staging Vertex Buffer using VMA
    vmaDestroyBuffer(vmaAllocator, stagingVertexBuffer, stagingVertexBufferAllocation);

    // Create Staging Buffer for Indices using VMA
    VkBuffer stagingIndexBuffer;
    VmaAllocation stagingIndexBufferAllocation;
    createBufferVMA(vmaAllocator, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |  VMA_ALLOCATION_CREATE_MAPPED_BIT,
                    stagingIndexBuffer, stagingIndexBufferAllocation, allocationInfo);

    // Copy Index Data to Staging Buffer using VMA mapping
    vmaMapMemory(vmaAllocator, stagingIndexBufferAllocation, &data);
    memcpy(data, indices.data(), (size_t) indexBufferSize);
    vmaUnmapMemory(vmaAllocator, stagingIndexBufferAllocation);

    // Create Index Buffer (Device Local) using VMA
    createBufferVMA(vmaAllocator, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_AUTO,
                    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, renderData.indexBuffer, renderData.indexBufferAllocation, allocationInfo);
    
    vkUtils::SetVulkanObjectName(disp, (uint64_t) renderData.indexBuffer, VK_OBJECT_TYPE_BUFFER, "Index Buffer");

    // Copy from Staging Index Buffer to Device Local Index Buffer
    copyBuffer(disp, queue, command_pool, stagingIndexBuffer, renderData.indexBuffer, indexBufferSize);

    // Clean up Staging Index Buffer using VMA
    vmaDestroyBuffer(vmaAllocator, stagingIndexBuffer, stagingIndexBufferAllocation);
}

void utils::MemoryUtils::createMaterialParamsBuffer(vulkan::DeviceManager& device_manager, const std::unordered_map<uint32_t, MaterialParams>& materialParams)
{
    uint32_t maxMaterialIndex = 0;
    if (!materialParams.empty())
    {
        // Find the element with the maximum key (material index)
        auto max_it = std::ranges::max_element(materialParams,
                                               [](const auto& a, const auto& b)
                                               {
                                                   return a.first < b.first;
                                               });
        maxMaterialIndex = max_it->first;
    }

    // The vector size needs to be max_index + 1 to accommodate all indices from 0 to max_index
    size_t materialCount = maxMaterialIndex + 1;
    std::vector<MaterialParams> materialData(materialCount);

    // Populate the vector using the map, placing each material at its correct index
    for (const auto& pair : materialParams)
    {
        if (pair.first < materialCount)
        {
            materialData[pair.first] = pair.second;
        }
        else
        {
            std::cerr << "Warning: Material index " << pair.first << " is out of expected range (" << materialCount << ")" << std::endl;
        }
    }

    VkDeviceSize bufferSize = sizeof(MaterialParams) * materialData.size();
    DescriptorUtils::createBuffer(device_manager.getAllocator(), bufferSize, materialValues.materialsBuffer);
    
    mapPersistenData(device_manager.getAllocator(), renderData.materialValues.materialsBuffer.allocation, renderData.materialValues.materialsBuffer.allocationInfo, materialData.data(), bufferSize);
}

VkBool32 utils::MemoryUtils::getSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat)
{
    std::vector<VkFormat> formatList =
    {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
    };

    for (auto& format : formatList)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            *depthStencilFormat = format;
            return true;
        }
    }

    return false;
}

void utils::MemoryUtils::setupDepthStencil(vkb::DispatchTable disp, VkExtent2D extents, VmaAllocator allocator, DepthStencil_Image& depthImage)
{
    VkImageCreateInfo imageCI{};
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = depthImage.format;
    imageCI.extent = { extents.width, extents.height, 1 };
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(allocator, &imageCI, &allocInfo, &depthImage.image, &depthImage.allocation, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create depth stencil image!");
    }

    VkImageViewCreateInfo imageViewCI{};
    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.image = depthImage.image;
    imageViewCI.format = depthImage.format;
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = 1;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = 1;
    imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    
    // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
    if (depthImage.format >= VK_FORMAT_D16_UNORM_S8_UINT)
    {
        imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    if (disp.createImageView(&imageViewCI, nullptr,  &depthImage.view) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create depth stencil image view!");
    }
}