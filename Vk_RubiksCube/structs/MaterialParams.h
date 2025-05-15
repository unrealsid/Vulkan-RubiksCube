#pragma once
#include <glm/vec4.hpp>

struct MaterialParams
{
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 shininess;
    
    glm::vec4 emissive;

    glm::vec4 alpha;
};
