#pragma once

#include <unordered_map>
#include "../structs/Vk_RenderData.h"

class Entity
{
public:
    Entity(RenderData render_data, std::unordered_map<uint32_t, std::pair<size_t, size_t>> material_index_range) :
        material_index_ranges(std::move(material_index_range)),
        render_data(std::move(render_data)), model()
    {
    }

    glm::mat4 get_model_matrix() const { return model; }
    RenderData get_render_data() const { return render_data; }
    std::unordered_map<uint32_t, std::pair<size_t, size_t>> get_material_index_range() const { return material_index_ranges; }
    
private:

    //A map of material index to indices range
    std::unordered_map<uint32_t, std::pair<size_t, size_t>> material_index_ranges;
    RenderData render_data;
    glm::mat4 model;
};
