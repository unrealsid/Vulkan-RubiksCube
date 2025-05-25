#pragma once
#include "VkBootstrapDispatch.h"
#include "../structs/Vk_RenderData.h"
#include "../structs/Vk_DepthStencil_Image.h"

namespace vulkan
{
    class DeviceManager;
}

struct Init;
struct Vertex;
struct VMAAllocator;

namespace utils
{
    class MemoryUtils
    {
    public: 
        static void createVmaAllocator(vulkan::DeviceManager& device_manager);

        static void createBufferVMA(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
                             VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags vmaAllocationFlags, VkBuffer& buffer, VmaAllocation& allocation, VmaAllocationInfo
                             & allocationInfo);

        static VkResult mapPersistenData(VmaAllocator vmaAllocator, VmaAllocation allocation, const VmaAllocationInfo& allocationInfo, const void* data, VkDeviceSize bufferSize);

        static VkDeviceAddress getBufferDeviceAddress(const vkb::DispatchTable& disp, VkBuffer buffer);

        static void copyBuffer(vkb::DispatchTable disp, VkQueue queue, VkCommandPool command_pool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        static void createVertexAndIndexBuffersVMA(VmaAllocator vmaAllocator, vkb::DispatchTable disp, VkQueue queue, VkCommandPool command_pool,RenderData& renderData, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        static VkBool32 getSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat);

        static void setupDepthStencil(vkb::DispatchTable disp, VkExtent2D extents, VmaAllocator
                               allocator, DepthStencil_Image& depthImage);

        static void createMaterialParamsBuffer(vulkan::DeviceManager& device_manager, const std::unordered_map<uint32_t, MaterialParams>& materialParams);
    };
}
