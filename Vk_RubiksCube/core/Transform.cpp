#include "Transform.h"

glm::mat4 Transform::get_model_matrix()
{
    model_matrix = glm::translate(glm::mat4(1.0f), position);
    model_matrix *= glm::toMat4(rotation);
    model_matrix = glm::scale(model_matrix, scale);
    return model_matrix;
}

void Transform::set_rotation(const glm::vec3& eulerAngle)
{
    glm::vec3 eulerAnglesRadians = glm::radians(eulerAngle);
    rotation = glm::quat(eulerAnglesRadians);
}