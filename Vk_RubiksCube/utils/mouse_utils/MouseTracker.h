#pragma once
#include <GLFW/glfw3.h>
#include <cstdlib>
#include "../../enums/MouseDirection.h"

namespace utils
{
    class MouseTracker
    {
    private:
        double last_x, last_y;
        double current_x, current_y;
        bool first_mouse;
        double threshold;
    
    public:
        MouseTracker(double thresh = 5.0);
    
        void update_position(GLFWwindow* window);
        MouseDirection get_direction() const;
        void get_movement_delta(double& delta_x, double& delta_y);
        void commit_position();
        double get_movement_speed();
        void reset();
    };

    inline const char* direction_to_string(MouseDirection dir)
    {
        switch (dir)
        {
            case MouseDirection::none: return "none";
            case MouseDirection::left: return "left";
            case MouseDirection::right: return "right";
            case MouseDirection::up: return "up";
            case MouseDirection::down: return "down";
            case MouseDirection::up_left: return "up_left";
            case MouseDirection::up_right: return "up_right";
            case MouseDirection::down_left: return "down_left";
            case MouseDirection::down_right: return "down_right";
        }
    
        return "unknown";
    }
}