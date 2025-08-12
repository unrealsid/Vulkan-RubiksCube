#pragma once

//Stores data for a single triangle in a compute shader
struct TriangleInfo
{
    alignas(16) glm::vec4 vertex1;
    alignas(16) glm::vec4 vertex2;
    alignas(16) glm::vec4 vertex3;
    alignas(16) glm::vec4 normal;

    uint64_t world_transform_id;
    uint64_t object_id;
};
