#include "VMA_MemoryUtils.h"
#include <iostream>
#include "../structs/Vk_RenderData.h"
#include "../structs/Vertex.h"
#include "../structs/Vk_Init.h"

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>


void vmaUtils::createVmaAllocator(Init& init)
{
    PFN_vkGetInstanceProcAddr fnGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(vkGetInstanceProcAddr);
    PFN_vkGetDeviceProcAddr fnGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(vkGetDeviceProcAddr);
    
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = fnGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = fnGetDeviceProcAddr;
   
    static VmaAllocatorCreateInfo allocatorInfo;

    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorInfo.physicalDevice = init.device.physical_device;
    allocatorInfo.instance = init.instance;
    allocatorInfo.device = init.device.device;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;
    
    VkResult result = vmaCreateAllocator(&allocatorInfo, &init.vmaAllocator);
    if (result != VK_SUCCESS)
    {
        std::cerr << "Failed to create VMA allocator: " << result << std::endl;
        throw std::runtime_error("Failed to create VMA allocator!");
    }

    std::cout << "VMA allocator created successfully." << std::endl;
}

void vmaUtils::createBufferVMA(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
                               VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags vmaAllocationFlags, VkBuffer& buffer, VmaAllocation& allocation)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = memoryUsage;  
    allocCreateInfo.flags = vmaAllocationFlags;

    if (VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &allocation, nullptr); result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer with VMA!");
    }
}

void vmaUtils::copyBuffer(vkb::DispatchTable disp, VkQueue queue, VkCommandPool command_pool, VkBuffer srcBuffer,
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

void vmaUtils::createVertexAndIndexBuffersVMA(VmaAllocator vmaAllocator, vkb::DispatchTable disp, VkQueue queue, VkCommandPool command_pool, RenderData& renderData, const
                                              std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
    VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();

    // Create Staging Buffer for Vertices using VMA
    VkBuffer stagingVertexBuffer;
    VmaAllocation stagingVertexBufferAllocation;
    createBufferVMA(vmaAllocator, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingVertexBuffer, stagingVertexBufferAllocation);

    // Copy Vertex Data to Staging Buffer using VMA mapping
    void* data;
    vmaMapMemory(vmaAllocator, stagingVertexBufferAllocation, &data);
    memcpy(data, vertices.data(), (size_t) vertexBufferSize);
    vmaUnmapMemory(vmaAllocator, stagingVertexBufferAllocation);

    // Create Vertex Buffer (Device Local) using VMA
    createBufferVMA(vmaAllocator, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,renderData.vertexBuffer, renderData.vertexBufferAllocation);

    // Copy from Staging Vertex Buffer to Device Local Vertex Buffer
    copyBuffer(disp, queue, command_pool, stagingVertexBuffer, renderData.vertexBuffer, vertexBufferSize);

    // Clean up Staging Vertex Buffer using VMA
    vmaDestroyBuffer(vmaAllocator, stagingVertexBuffer, stagingVertexBufferAllocation);


    // Create Staging Buffer for Indices using VMA
    VkBuffer stagingIndexBuffer;
    VmaAllocation stagingIndexBufferAllocation;
    createBufferVMA(vmaAllocator, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingIndexBuffer, stagingIndexBufferAllocation);

    // Copy Index Data to Staging Buffer using VMA mapping
    vmaMapMemory(vmaAllocator, stagingIndexBufferAllocation, &data);
    memcpy(data, indices.data(), (size_t) indexBufferSize);
    vmaUnmapMemory(vmaAllocator, stagingIndexBufferAllocation);

    // Create Index Buffer (Device Local) using VMA
    createBufferVMA(vmaAllocator, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, renderData.indexBuffer, renderData.indexBufferAllocation);

    // Copy from Staging Index Buffer to Device Local Index Buffer
    copyBuffer(disp, queue, command_pool, stagingIndexBuffer, renderData.indexBuffer, indexBufferSize);

    // Clean up Staging Index Buffer using VMA
    vmaDestroyBuffer(vmaAllocator, stagingIndexBuffer, stagingIndexBufferAllocation);
}

VkPhysicalDeviceBufferDeviceAddressFeatures vmaUtils::create_physical_device_buffer_address()
{
    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = 
    {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
        nullptr,
        VK_TRUE // Enable the bufferDeviceAddress feature
    };

    return bufferDeviceAddressFeatures;
}
