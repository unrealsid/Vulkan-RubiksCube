#pragma once

#ifndef VK_RENDER_DATA_H
#define VK_RENDER_DATA_H

#include "GPU_Buffer.h"
#include "Vertex.h"
#include <vector>
#include <unordered_map>

struct RenderData
{
    GPU_Buffer vertex_buffer;
    GPU_Buffer index_buffer;

    glm::vec3 local_position;
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> material_index_ranges;
};

#endif