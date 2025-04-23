#pragma once
#include <string>
#include <vector>
#include "../structs/Vertex.h"

namespace vkUtils
{
    bool loadModel(const std::string& modelPath, std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices);
}

