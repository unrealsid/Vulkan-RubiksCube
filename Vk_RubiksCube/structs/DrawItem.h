#pragma once
#include "GPU_Buffer.h"

struct DrawItem
{
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    uint32_t index_count;
};

