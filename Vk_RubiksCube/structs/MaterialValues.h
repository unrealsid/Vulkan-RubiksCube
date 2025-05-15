#pragma once
#include <vector>

#include "Buffer.h"
#include "MaterialParams.h"

struct MaterialValues
{
    std::vector<MaterialParams> materialParams;

    Buffer materialsBuffer;
    VkDeviceAddress materialParamsBufferAddress;
};
