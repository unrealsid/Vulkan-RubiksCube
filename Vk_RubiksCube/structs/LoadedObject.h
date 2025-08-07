#pragma once

#include <string>
#include <vector>
#include "Vertex.h"

struct LoadedObject
{
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    GPU_Buffer vertex_buffer = {};
    GPU_Buffer index_buffer = {};

    //A map of material ID to index range for rendering 
    std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> material_index_ranges;

    //This is the averaged location by computing the average from the vertices provided by tinyobj loader 
    glm::vec3 local_position;
};
