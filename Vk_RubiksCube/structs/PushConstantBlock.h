#pragma once
#include <vulkan_core.h>

struct PushConstantBlock
{
    VkDeviceAddress scene_buffer_address;
    VkDeviceAddress material_params_address;
    VkDeviceAddress object_model_transform_addr;
};