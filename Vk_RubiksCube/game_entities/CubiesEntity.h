#pragma once

#include "../core/DrawableEntity.h"

class CubiesEntity : public core::DrawableEntity
{
public:
    CubiesEntity(uint32_t entity_id, const RenderData& render_data, EngineContext& engine_context,
        const std::string& entity_string_id)
        : DrawableEntity(entity_id, render_data, engine_context, entity_string_id)
    {
        can_rotate = false;
        use_trs_matrix = true;
        start_model_matrix = glm::mat4{};
        face_rotation_speed = 1.0f;
    }

    void reset_entity();

    void set_rotation_params(bool can_rotate, const glm::vec3& new_rotation_axis, float rotation_angle);

    [[nodiscard]] bool is_rotating() const{ return can_rotate; }

private:
    glm::mat4 new_rotation_matrix;
    float rotation_angle;
    glm::vec3 rotation_axis;
    bool can_rotate;

private:
    glm::vec3 inital_position;
    float angle;

    glm::mat4 start_model_matrix;

    bool use_trs_matrix;
    glm::vec3 initial_rotation;
    glm::mat4 initial_model_matrix;
    
    float face_rotation_speed;

    void stop_rotation();
    
    virtual void initialize_transform() const override;
    void update(double delta_time) override;
};
