#pragma once

#include "../core/DrawableEntity.h"

class DynamicRootEntity : public core::DrawableEntity
{
public:
    DynamicRootEntity(uint32_t entity_id, const RenderData& render_data, EngineContext& engine_context,
        const std::string& entity_string_id)
        : DrawableEntity(entity_id, render_data, engine_context, entity_string_id)
    {
        can_rotate = false;
        rotation_speed = 0.0;
    }
    
    bool can_rotate;
    glm::mat4 new_rotation_matrix;
    float rotation_angle;
    glm::vec3 rotation_axis;

    void reset_entity();

    void set_rotation_params(bool can_rotate, const glm::vec3& new_rotation_axis, float rotation_angle);
private:
    float rotation_speed;
    
    virtual void update(double delta_time) override;

    virtual void initialize_transform() const override;

};
