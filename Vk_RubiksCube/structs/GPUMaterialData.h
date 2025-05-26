#pragma once
#include <vector>

#include "Vk_Buffer.h"
#include "MaterialParams.h"

struct GPUMaterialData
{
    std::vector<MaterialParams> materialParams;

    Vk_Buffer materialsBuffer;
    VkDeviceAddress materialParamsBufferAddress;
};
