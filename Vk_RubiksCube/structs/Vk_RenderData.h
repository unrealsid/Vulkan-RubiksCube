#pragma once

#ifndef VK_RENDER_DATA_H
#define VK_RENDER_DATA_H

#include "GPU_Buffer.h"
#include "Vertex.h"
#include <vector>

struct RenderData
{
    GPU_Buffer vertex_buffer;
    GPU_Buffer index_buffer;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

#endif