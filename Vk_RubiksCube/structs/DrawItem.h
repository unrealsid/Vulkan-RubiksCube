#pragma once
#include "GPU_Buffer.h"

struct DrawItem
{
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    std::pair<uint32_t, uint32_t> index_range;
    uint32_t index_count;
};

