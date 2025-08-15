#pragma once
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "../../structs/Vk_SceneData.h"

class OrbitCamera
{
public:
    public:
    OrbitCamera(const glm::vec3& target = glm::vec3(0.0f),
                float distance = 5.0f,
                float theta = 0.0f,
                float phi = glm::pi<float>() * 0.25f);

    // Update camera based on mouse input
    void update_rotation(float delta_x, float delta_y, float sensitivity = 0.005f);
    void update_distance(float delta_distance);
    void update_target(const glm::vec3& new_target);

    // Matrix calculations
    glm::mat4 get_view_matrix() const;
    glm::mat4 get_projection_matrix(float aspect_ratio, float fov = glm::radians(45.0f), 
                                   float near_plane = 0.1f, float far_plane = 100.0f) const;

    Vk_SceneData get_scene_data(float aspect_ratio) const
    {
        return
        {
            get_view_matrix(),
            get_projection_matrix(aspect_ratio)
        };
    }

    // Getters
    glm::vec3 get_position() const;
    glm::vec3 get_target() const { return m_target; }
    glm::vec3 get_forward() const;
    glm::vec3 get_right() const;
    glm::vec3 get_up() const;
    float get_distance() const { return m_distance; }
    float get_theta() const { return m_theta; }
    float get_phi() const { return m_phi; }

    // Setters
    void set_target(const glm::vec3& target) { m_target = target; }
    void set_distance(float distance);
    void set_angles(float theta, float phi);
    void set_distance_limits(float min_dist, float max_dist);
    void set_phi_limits(float min_phi, float max_phi);

    // Ray casting for mouse picking
    glm::vec3 screen_to_world_ray(float screen_x, float screen_y, 
                                 float screen_width, float screen_height,
                                 float aspect_ratio) const;

    // Reset camera to default position
    void reset();

private:
    void update_position();
    void clamp_angles();

    glm::vec3 m_target;
    float m_distance;
    float m_theta;  // Horizontal angle (azimuth)
    float m_phi;    // Vertical angle (elevation)
    
    glm::vec3 m_position;
    
    // Limits
    float m_min_distance = 0.5f;
    float m_max_distance = 50.0f;
    float m_min_phi = 0.01f;  // Prevent gimbal lock
    float m_max_phi = glm::pi<float>() - 0.01f;
    
    // Default values for reset
    glm::vec3 m_default_target;
    float m_default_distance;
    float m_default_theta;
    float m_default_phi;
};
