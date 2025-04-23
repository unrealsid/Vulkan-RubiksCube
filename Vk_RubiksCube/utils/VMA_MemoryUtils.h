#pragma once
#include "VkBootstrapDispatch.h"
#include "../structs/Vk_RenderData.h"



struct Init;
struct Vertex;
struct VMAAllocator;

namespace vmaUtils
{
    void createVmaAllocator(Init& init);

    void createBufferVMA(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
                         VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags vmaAllocationFlags, VkBuffer& buffer, VmaAllocation& allocation);

    void copyBuffer(vkb::DispatchTable disp, VkQueue queue, VkCommandPool command_pool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createVertexAndIndexBuffersVMA(VmaAllocator vmaAllocator, vkb::DispatchTable disp, VkQueue queue, VkCommandPool command_pool,RenderData& renderData, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    VkPhysicalDeviceBufferDeviceAddressFeatures create_physical_device_buffer_address();
}
