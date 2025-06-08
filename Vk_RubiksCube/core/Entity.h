#pragma once

#include <unordered_map>
#include "../structs/Vk_RenderData.h"

class Entity
{
public:
    Entity(RenderData render_data) :
        render_data(std::move(render_data)), model()
    {
    }

    glm::mat4 get_model_matrix() const { return model; }
    RenderData get_render_data() const { return render_data; }
    
private:

    RenderData render_data;
    glm::mat4 model;
};
