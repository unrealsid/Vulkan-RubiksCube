#pragma once

#include <array>
#include <vulkan_core.h>
#include <glm/glm.hpp>

struct Vertex_ObjectPicking
{
    glm::vec3 position;
    glm::vec3 normal;

    bool operator==(const Vertex_ObjectPicking& other) const
    {
        return position == other.position && normal == other.normal;
    }

    static VkVertexInputBindingDescription2EXT get_binding_description()
    {
        VkVertexInputBindingDescription2EXT bindingDescription{};
        bindingDescription.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
        bindingDescription.pNext = nullptr;
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex); //Important to use thisi stride because we ultimately use the first buffer to read vertex data.
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.divisor = 1;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription2EXT, 2> get_attribute_descriptions()
    {
        std::array<VkVertexInputAttributeDescription2EXT, 2> attribute_descriptions{};

        // Position attribute
        attribute_descriptions[0].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
        attribute_descriptions[0].pNext = nullptr;
        attribute_descriptions[0].location = 0; // Corresponds to layout(location = 0) in shader
        attribute_descriptions[0].binding = 0; // Corresponds to binding point 0
        attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex_ObjectPicking, position); // Offset within the Vertex struct

        attribute_descriptions[1].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
        attribute_descriptions[1].pNext = nullptr;
        attribute_descriptions[1].location = 1; 
        attribute_descriptions[1].binding = 0; 
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex_ObjectPicking, normal); 

        return attribute_descriptions;
    }
};