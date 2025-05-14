#pragma once
#include <vk_mem_alloc.h>
#include <vulkan_core.h>
#include "BufferInfo.h"

struct DescriptorInfo
{
    VkDeviceSize layoutOffset;
    VkDeviceSize layoutSize;
    VkDescriptorSetLayout setLayout;
    VkDeviceOrHostAddressConstKHR bufferDeviceAddress;
    
    BufferInfo bufferInfo;
};
