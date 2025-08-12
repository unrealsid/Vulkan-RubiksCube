#pragma once
#include <vulkan_core.h>

struct PushConstantBlock
{
    VkDeviceAddress scene_buffer_address;
    VkDeviceAddress material_params_address;
    VkDeviceAddress object_model_transform_addr;
};

struct ObjectPickerPushConstantBlock
{
    VkDeviceAddress scene_buffer_addr;
    VkDeviceAddress model_transform_addr;
    VkDeviceAddress object_id_addr;
    VkDeviceAddress face_normal_addr;
};

struct HitTraceConstantBlock
{
    VkDeviceAddress scene_buffer_addr;
    VkDeviceAddress world_transform_buffer_addr;
    VkDeviceAddress mesh_triangle_buffer_addr;
    VkDeviceAddress ray_buffer_addr;
    VkDeviceAddress output_buffer_addr;
};