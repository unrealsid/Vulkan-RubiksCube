#include "CubiesEntity.h"
#include <glm/gtx/string_cast.hpp>
#include "../vulkan/DeviceManager.h"

void CubiesEntity::reset_entity()
{
}

void CubiesEntity::set_rotation_params(bool can_rotate, const glm::vec3& new_rotation_axis, float rotation_angle)
{
    this->can_rotate = can_rotate;
    this->rotation_axis = new_rotation_axis;
    this->rotation_angle = glm::radians(rotation_angle);
    initial_model_matrix = get_transform()->get_model_matrix(); 
    
    angle = 0; 
    use_trs_matrix = false;
}

void CubiesEntity::initialize_transform() const
{
    DrawableEntity::initialize_transform();
}

void CubiesEntity::update(double delta_time)
{
    if(can_rotate)
    {
        if(rotation_angle > 0)
        {
            angle +=  face_rotation_speed * (float) delta_time;
        }
        else
        {
            angle -= face_rotation_speed * (float) delta_time;
        }
        
        glm::mat4 incremental_rotation = glm::rotate(glm::mat4(1.0f), angle, rotation_axis);

        // 2. Apply the new rotation to the cubie's state BEFORE this turn started
        glm::mat4 model_matrix = incremental_rotation * initial_model_matrix;

        // 3. Decompose the final matrix to update the transform component
        // This ensures the transform always has the correct world position/rotation
        get_transform()->set_position(glm::vec3(model_matrix[3]));
        glm::quat rotation_quat = glm::quat_cast(model_matrix);
        get_transform()->set_rotation(glm::degrees(glm::eulerAngles(rotation_quat)));

        update_transform_on_gpu(model_matrix);
        
        stop_rotation();
    }
    else
    {
        if(use_trs_matrix)
        {
            DrawableEntity::update(delta_time);
        }
    }
}

void CubiesEntity::stop_rotation()
{
    if((rotation_angle > 0 && angle >= rotation_angle) ||
       (rotation_angle < 0) && (angle <= rotation_angle))
    {
        can_rotate = false;

        //Snap to the final position
        glm::mat4 final_rotation = glm::rotate(glm::mat4(1.0f), rotation_angle, rotation_axis);
        glm::mat4 final_model_matrix = final_rotation * initial_model_matrix;

        // Update transform component one last time
        get_transform()->set_position(glm::vec3(final_model_matrix[3]));
        glm::quat rotation_quat = glm::quat_cast(final_model_matrix);
        get_transform()->set_rotation(glm::degrees(glm::eulerAngles(rotation_quat)));
        
        update_transform_on_gpu(final_model_matrix);
        
        //std::cout << "cubie id " <<  get_entity_id() << "\n" << *get_transform() << std::endl; 
    }
}
