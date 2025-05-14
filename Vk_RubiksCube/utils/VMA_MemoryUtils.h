#pragma once
#include "VkBootstrapDispatch.h"
#include "../structs/Vk_RenderData.h"
#include "../structs/Vk_DepthStencil_Image.h"

struct Init;
struct Vertex;
struct VMAAllocator;

namespace vmaUtils
{
    void createVmaAllocator(Init& init);

    void createBufferVMA(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
                         VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags vmaAllocationFlags, VkBuffer& buffer, VmaAllocation& allocation, VmaAllocationInfo
                         & allocationInfo);

    VkResult mapPersistenData(VmaAllocator vmaAllocator, VmaAllocation allocation, const VmaAllocationInfo& allocationInfo, const void* data, VkDeviceSize bufferSize);

    VkDeviceAddress getBufferDeviceAddress(const vkb::DispatchTable& disp, VkBuffer buffer);

    void copyBuffer(vkb::DispatchTable disp, VkQueue queue, VkCommandPool command_pool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createVertexAndIndexBuffersVMA(VmaAllocator vmaAllocator, vkb::DispatchTable disp, VkQueue queue, VkCommandPool command_pool,RenderData& renderData, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    VkBool32 getSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat);

    void setupDepthStencil(vkb::DispatchTable disp, VkExtent2D extents, VmaAllocator
                           allocator, DepthStencil_Image& depthImage);

    VkPhysicalDeviceBufferDeviceAddressFeatures create_physical_device_buffer_address();
}
