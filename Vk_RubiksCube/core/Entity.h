#pragma once

#include <unordered_map>
#include "../structs/Vk_RenderData.h"

typedef std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> MaterialIndexRange;

class Entity
{
public:
    Entity(RenderData render_data, MaterialIndexRange material_index_range) :
        material_index_ranges(std::move(material_index_range)),
        render_data(std::move(render_data)), model()
    {
    }

    glm::mat4 get_model_matrix() const { return model; }
    RenderData get_render_data() const { return render_data; }
    MaterialIndexRange get_material_index_range() const { return material_index_ranges; }
    
private:

    //A map of material index to indices range
    MaterialIndexRange material_index_ranges;
    RenderData render_data;
    glm::mat4 model;
};
