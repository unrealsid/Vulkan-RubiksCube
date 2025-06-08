#pragma once
#include <vector>

#include "GPU_Buffer.h"
#include "MaterialParams.h"

struct Vk_MaterialData
{
    std::vector<MaterialParams> material_params;

    GPU_Buffer materials_buffer;
    VkDeviceAddress material_params_buffer_address;
};
