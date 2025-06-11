#pragma once

#define GLM_FORCE_CXX17
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform
{
public:
    Transform() : position(0.0f), rotation(glm::quat()), scale(1.0f), model_matrix(glm::mat4(1.0f))
    {
    }

    glm::mat4 get_model_matrix();

    glm::vec3 get_position() const { return position; }
    void set_position(const glm::vec3& position) { this->position = position;  }

    void set_rotation(const glm::vec3& eulerAngle);
    
    glm::quat get_rotation_quat() const{ return rotation; }
    glm::vec3 get_rotation_euler() const { return glm::eulerAngles(rotation); }

    glm::vec3 get_scale() const{ return scale; }
    void set_scale(const glm::vec3& scale) { this->scale = scale; }

private:    
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;

    glm::mat4 model_matrix;
};