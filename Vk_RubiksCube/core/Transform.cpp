#include "Transform.h"

glm::mat4 Transform::get_model_matrix() const
{
    if (parent)
    {
        return parent->get_model_matrix() * get_local_matrix();
    }
        
    return get_local_matrix();
}

glm::mat4 Transform::get_local_matrix() const
{
    glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 R = glm::toMat4(rotation);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
    return T * R * S;
}

glm::vec3 Transform::get_world_position() const
{
   return glm::vec3(get_model_matrix()[3]);
}

void Transform::set_rotation(const glm::vec3& euler_angle)
{
    glm::vec3 eulerAnglesRadians = glm::radians(euler_angle);
    rotation = glm::quat(eulerAnglesRadians);
}
