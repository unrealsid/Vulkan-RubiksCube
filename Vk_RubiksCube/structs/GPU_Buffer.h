#pragma once
#include <vk_mem_alloc.h>
#include <vulkan_core.h>

struct GPU_Buffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo allocation_info;
    VkDeviceAddress buffer_address;
};
