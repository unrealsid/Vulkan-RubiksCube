#include "DynamicRootEntity.h"

#include <glm/gtx/string_cast.hpp>


void DynamicRootEntity::reset_entity()
{
    auto rotation = glm::degrees(get_transform()->get_rotation_euler());

    if(rotation.x > 90.0f || rotation.y > 90.f || rotation.z > 90 ||
       rotation.x < -90.0f || rotation.y < -90.f || rotation.z < -90)
    {
        can_rotate = false;
    }
    
    std::cout << glm::to_string(rotation) << std::endl; 
}

void DynamicRootEntity::set_rotation_params(bool can_rotate, const glm::vec3& new_rotation_axis, float rotation_angle)
{
    this->can_rotate = can_rotate;
    this->rotation_axis = new_rotation_axis;
    this->rotation_angle = rotation_angle;
    rotation_speed = 0.0;
}

void DynamicRootEntity::update(double delta_time)
{
    if(can_rotate)
    {
        rotation_speed += 0.005f * delta_time;
        auto angle =  rotation_speed * rotation_angle;
        std::cout << angle << "\n";

        new_rotation_matrix = glm::rotate(glm::mat4(1.0f), angle, rotation_axis);
        
        glm::quat q = glm::quat_cast(new_rotation_matrix);           // matrix → quaternion
        glm::vec3 euler = glm::eulerAngles(q);                       // quaternion → radians
        euler = glm::degrees(euler);                                 // convert to degrees
        
        get_transform()->set_rotation(euler);
        
        reset_entity();
    }
    
    DrawableEntity::update(delta_time);
}

void DynamicRootEntity::initialize_transform() const
{
    DrawableEntity::initialize_transform();
    //transform->set_scale(glm::vec3(0.5f));
}
