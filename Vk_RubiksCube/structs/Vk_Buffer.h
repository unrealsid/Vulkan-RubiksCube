#pragma once
#include <vk_mem_alloc.h>
#include <vulkan_core.h>

struct Vk_Buffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo allocationInfo;
};
