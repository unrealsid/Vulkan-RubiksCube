#pragma once

#include <array>
#include <vulkan_core.h>
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    uint32_t  materialIndex;
    uint32_t  textureIndex;

    bool operator==(const Vertex& other) const
    {
        return position == other.position && normal == other.normal && texCoords == other.texCoords && materialIndex == other.materialIndex && textureIndex == other.textureIndex;
    }

    static VkVertexInputBindingDescription2EXT get_binding_description()
    {
        VkVertexInputBindingDescription2EXT bindingDescription{};
        bindingDescription.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
        bindingDescription.pNext = nullptr;
        bindingDescription.binding = 0; 
        bindingDescription.stride = sizeof(Vertex); 
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.divisor = 1;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription2EXT, 5> get_attribute_descriptions()
    {
        std::array<VkVertexInputAttributeDescription2EXT, 5> attribute_descriptions{};

        // Position attribute
        attribute_descriptions[0].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
        attribute_descriptions[0].pNext = nullptr;
        attribute_descriptions[0].location = 0; // Corresponds to layout(location = 0) in shader
        attribute_descriptions[0].binding = 0; // Corresponds to binding point 0
        attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // glm::vec3 is 3 floats
        attribute_descriptions[0].offset = offsetof(Vertex, position); // Offset within the Vertex struct

        // Normal attribute
        attribute_descriptions[1].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
        attribute_descriptions[1].pNext = nullptr;
        attribute_descriptions[1].location = 1; // Corresponds to layout(location = 1) in shader
        attribute_descriptions[1].binding = 0; // Corresponds to binding point 0
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // glm::vec3 is 3 floats
        attribute_descriptions[1].offset = offsetof(Vertex, normal); // Offset within the Vertex struct

        // Texture Coordinate attribute
        attribute_descriptions[2].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
        attribute_descriptions[2].pNext = nullptr;
        attribute_descriptions[2].location = 2; // Corresponds to layout(location = 2) in shader
        attribute_descriptions[2].binding = 0; // Corresponds to binding point 0
        attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT; // glm::vec2 is 2 floats
        attribute_descriptions[2].offset = offsetof(Vertex, texCoords); // Offset within the Vertex struct

        attribute_descriptions[3].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
        attribute_descriptions[3].binding = 0; 
        attribute_descriptions[3].location = 3; // Shader location
        attribute_descriptions[3].format = VK_FORMAT_R32_UINT; // uint32_t
        attribute_descriptions[3].offset = offsetof(Vertex, materialIndex);

        // Texture index attribute
        attribute_descriptions[4].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
        attribute_descriptions[4].pNext = nullptr;
        attribute_descriptions[4].binding = 0; 
        attribute_descriptions[4].location = 4; // Shader location
        attribute_descriptions[4].format = VK_FORMAT_R32_UINT; // uint32_t
        attribute_descriptions[4].offset = offsetof(Vertex, textureIndex);

        return attribute_descriptions;
    }
};

// Custom hash function for Vertex, needed for std::unordered_map
namespace std
{
    template<> struct hash<Vertex>
    {
        size_t operator()(const Vertex& vertex) const noexcept
        {
            // Simple hash combining function for GLM vectors
            auto hashVec3 = [](const glm::vec3& v)
            {
                return hash<float>()(v.x) ^ (hash<float>()(v.y) << 1) ^ (hash<float>()(v.z) << 2);
            };
            auto hashVec2 = [](const glm::vec2& v)
            {
                return hash<float>()(v.x) ^ (hash<float>()(v.y) << 1);
            };

            size_t h1 = hashVec3(vertex.position);
            size_t h2 = hashVec3(vertex.normal);
            size_t h3 = hashVec2(vertex.texCoords);
            size_t h4 = hash<uint32_t>()(vertex.materialIndex);
            size_t h5 = hash<uint32_t>()(vertex.textureIndex);

            // Combine all hashes
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
        }
    };
}

