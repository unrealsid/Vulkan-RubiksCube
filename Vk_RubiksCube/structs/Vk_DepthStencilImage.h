#pragma once

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

struct DepthStencilImage
{
    VkImage image = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
};
