#include "Camera.h"

#include <algorithm>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <cmath>

OrbitCamera::OrbitCamera(const glm::vec3& target, float distance, float theta, float phi)
    : m_target(target)
    , m_distance(distance)
    , m_theta(theta)
    , m_phi(phi)
    , m_default_target(target)
    , m_default_distance(distance)
    , m_default_theta(theta)
    , m_default_phi(phi)
{
    clamp_angles();
    update_position();
}

void OrbitCamera::update_rotation(float delta_x, float delta_y, float sensitivity)
{
    //Change sign order here to change how the camera orbits
    m_theta += delta_x * sensitivity;  
    m_phi -= delta_y * sensitivity;    
    
    clamp_angles();
    update_position();
}

void OrbitCamera::update_distance(float delta_distance)
{
    m_distance += delta_distance;
    m_distance = std::clamp(m_distance, m_min_distance, m_max_distance);
    update_position();
}

void OrbitCamera::update_target(const glm::vec3& new_target)
{
    m_target = new_target;
    update_position();
}

glm::mat4 OrbitCamera::get_view_matrix() const
{
    return glm::lookAt(m_position, m_target, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 OrbitCamera::get_projection_matrix(float aspect_ratio, float fov, 
                                             float near_plane, float far_plane) const
{
    // Vulkan uses different coordinate system than OpenGL
    glm::mat4 proj = glm::perspective(fov, aspect_ratio, near_plane, far_plane);
    
    // Flip Y coordinate for Vulkan
    proj[1][1] *= -1;
    
    return proj;
}

glm::vec3 OrbitCamera::get_position() const
{
    return m_position;
}

glm::vec3 OrbitCamera::get_forward() const
{
    return glm::normalize(m_target - m_position);
}

glm::vec3 OrbitCamera::get_right() const
{
    glm::vec3 forward = get_forward();
    glm::vec3 world_up(0.0f, 1.0f, 0.0f);
    return glm::normalize(glm::cross(forward, world_up));
}

glm::vec3 OrbitCamera::get_up() const
{
    glm::vec3 forward = get_forward();
    glm::vec3 right = get_right();
    return glm::cross(right, forward);
}

void OrbitCamera::set_distance(float distance)
{
    m_distance = std::clamp(distance, m_min_distance, m_max_distance);
    update_position();
}

void OrbitCamera::set_angles(float theta, float phi)
{
    m_theta = theta;
    m_phi = phi;
    clamp_angles();
    update_position();
}

void OrbitCamera::set_distance_limits(float min_dist, float max_dist)
{
    m_min_distance = min_dist;
    m_max_distance = max_dist;
    m_distance = std::clamp(m_distance, m_min_distance, m_max_distance);
    update_position();
}

void OrbitCamera::set_phi_limits(float min_phi, float max_phi)
{
    m_min_phi = min_phi;
    m_max_phi = max_phi;
    clamp_angles();
    update_position();
}

glm::vec3 OrbitCamera::screen_to_world_ray(float screen_x, float screen_y,
                                           float screen_width, float screen_height,
                                           float aspect_ratio) const
{
    // Convert screen coordinates to NDC (-1 to 1)
    float ndc_x = (2.0f * screen_x) / screen_width - 1.0f;
    float ndc_y = 1.0f - (2.0f * screen_y) / screen_height;  // Flip Y for screen coordinates
    
    // Create ray in clip space
    glm::vec4 ray_clip(ndc_x, ndc_y, -1.0f, 1.0f);
    
    // Transform to eye space
    glm::mat4 proj_matrix = get_projection_matrix(aspect_ratio);
    glm::vec4 ray_eye = glm::inverse(proj_matrix) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
    
    // Transform to world space
    glm::mat4 view_matrix = get_view_matrix();
    glm::vec3 ray_world = glm::vec3(glm::inverse(view_matrix) * ray_eye);
    
    return glm::normalize(ray_world);
}

void OrbitCamera::reset()
{
    m_target = m_default_target;
    m_distance = m_default_distance;
    m_theta = m_default_theta;
    m_phi = m_default_phi;
    clamp_angles();
    update_position();
}

void OrbitCamera::update_position()
{
    // Convert spherical coordinates to cartesian
    float x = m_distance * std::sin(m_phi) * std::cos(m_theta);
    float y = m_distance * std::cos(m_phi);
    float z = m_distance * std::sin(m_phi) * std::sin(m_theta);
    
    m_position = m_target + glm::vec3(x, y, z);
}

void OrbitCamera::clamp_angles()
{
    // Wrap theta around 2π
    while (m_theta > 2.0f * glm::pi<float>())
    {
        m_theta -= 2.0f * glm::pi<float>();
    }
    while (m_theta < 0.0f)
    {
        m_theta += 2.0f * glm::pi<float>();
    }
    
    // Clamp phi to prevent gimbal lock
    m_phi = std::clamp(m_phi, m_min_phi, m_max_phi);
}