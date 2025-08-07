#pragma once
#include "GPU_Buffer.h"
#include "../core/Entity.h"

struct DrawItem
{
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    std::pair<uint32_t, uint32_t> index_range;
    uint32_t index_count;

    //Who does this drawcall belong to?
    core::Entity* entity = nullptr;
};

