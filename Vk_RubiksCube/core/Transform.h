#pragma once

#define GLM_FORCE_CXX17
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>

class Transform
{
public:
    Transform() : position(0.0f), rotation(glm::quat()), scale(1.0f), model_matrix(glm::mat4(1.0f))
    {
    }

    //Parent of this transform, if any
    Transform* parent = nullptr;

    //Get the final matrix for this object and all child objects
    glm::mat4 get_model_matrix() const;

    glm::mat4 get_local_matrix() const;
    
    glm::vec3 get_world_position() const;

    //Sets offset
    glm::vec3 get_position() const { return position; }
    void set_position(const glm::vec3& position) { this->position = position;  }

    void set_rotation(const glm::vec3& eulerAngle);
    
    glm::quat get_rotation_quat() const{ return rotation; }
    glm::vec3 get_rotation_euler() const { return glm::eulerAngles(rotation); }

    glm::vec3 get_scale() const{ return scale; }
    void set_scale(const glm::vec3& scale) { this->scale = scale; }

    friend std::ostream& operator<<(std::ostream& os, const Transform& transform)
    {
        glm::vec3 euler = glm::degrees(glm::eulerAngles(transform.rotation));
        auto world_position = transform.get_world_position();
        
        os << "Transform { "
           << "Position: (" << world_position.x << ", " << world_position.y << ", " << world_position.z << "), "
           << "Rotation: (" << euler.x << "°, " << euler.y << "°, " << euler.z << "°), "
           << "Scale: (" << transform.scale.x << ", " << transform.scale.y << ", " << transform.scale.z << ") }";
        return os;
    }

private:    
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;

    glm::mat4 model_matrix;
};