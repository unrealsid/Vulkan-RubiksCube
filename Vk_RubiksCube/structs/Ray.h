#pragma once
#include <glm/vec4.hpp>

struct Ray
{
    glm::vec3 origin;
    float padding0;
    
    glm::vec3 direction;
    float padding1;
    
    float t_min;
    glm::vec3 padding2;
    
    float t_max;
    glm::vec3 padding3;
};
