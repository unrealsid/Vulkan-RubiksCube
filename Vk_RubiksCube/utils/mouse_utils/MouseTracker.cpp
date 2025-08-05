#include "MouseTracker.h"

#include "MouseTracker.h"
#include <cmath>
#include <cstdlib>

utils::MouseTracker::MouseTracker(double thresh) : 
    last_x(0), last_y(0), current_x(0), current_y(0), 
    first_mouse(true), threshold(thresh) {}

void utils::MouseTracker::update_position(GLFWwindow* window)
{
    glfwGetCursorPos(window, &current_x, &current_y);
    
    if (first_mouse)
    {
        last_x = current_x;
        last_y = current_y;
        first_mouse = false;
    }
}

MouseDirection utils::MouseTracker::get_direction() const
{
    double delta_x = current_x - last_x;
    double delta_y = current_y - last_y;
    
    // Check if movement is significant enough
    if (std::abs(delta_x) < threshold && std::abs(delta_y) < threshold)
    {
        return MouseDirection::none;
    }
    
    // Determine primary direction
    bool moving_right = delta_x > threshold;
    bool moving_left = delta_x < -threshold;
    bool moving_down = delta_y > threshold;  // Y increases downward in screen coords
    bool moving_up = delta_y < -threshold;
    
    // Return diagonal directions first (more specific)
    if (moving_up && moving_left) return MouseDirection::up_left;
    if (moving_up && moving_right) return MouseDirection::up_right;
    if (moving_down && moving_left) return MouseDirection::down_left;
    if (moving_down && moving_right) return MouseDirection::down_right;
    
    // Return cardinal directions
    if (moving_left) return MouseDirection::left;
    if (moving_right) return MouseDirection::right;
    if (moving_up) return MouseDirection::up;
    if (moving_down) return MouseDirection::down;
    
    return MouseDirection::none;
}

void utils::MouseTracker::get_movement_delta(double& delta_x, double& delta_y)
{
    delta_x = current_x - last_x;
    delta_y = current_y - last_y;
}

void utils::MouseTracker::commit_position()
{
    last_x = current_x;
    last_y = current_y;
}

double utils::MouseTracker::get_movement_speed()
{
    double delta_x = current_x - last_x;
    double delta_y = current_y - last_y;
    return std::sqrt(delta_x * delta_x + delta_y * delta_y);
}

void utils::MouseTracker::reset()
{
    first_mouse = true;
}
