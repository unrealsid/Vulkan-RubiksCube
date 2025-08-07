#pragma once
#include <string>
#include <vector>
#include "../materials/Material.h"

struct DrawBatch
{
    std::string shader_name;
    material::Material* material;
    
    std::vector<struct DrawItem> items;
};
