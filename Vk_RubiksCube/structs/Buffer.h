#pragma once
#include <vk_mem_alloc.h>
#include <vulkan_core.h>

enum class DeviceAddress : uint64_t { Invalid = 0 };

struct Buffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo allocationInfo;
};
