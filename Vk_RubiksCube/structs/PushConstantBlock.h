#pragma once
#include <vulkan_core.h>

struct PushConstantBlock
{
    VkDeviceAddress sceneBufferAddress;
    VkDeviceAddress materialParamsAddress;
    VkDeviceAddress object_model_transform_addr;
};
