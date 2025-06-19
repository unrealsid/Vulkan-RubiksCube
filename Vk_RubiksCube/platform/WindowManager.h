#pragma once
#include <memory>
#include <vulkan_core.h>

struct EngineContext;
struct GLFWwindow;

namespace window
{
    constexpr int window_width = 1280;
    constexpr int window_height = 720;
    
    class WindowManager
    {
    public:
        WindowManager(EngineContext& engine_context);
        ~WindowManager();
        
        GLFWwindow* create_window_glfw(const char* windowName = "", bool resize = true);
        void destroy_window_glfw() const;
        VkSurfaceKHR create_surface_glfw(VkInstance instance, VkAllocationCallbacks* allocator = nullptr) const;

        GLFWwindow* get_window() const;

        void update_mouse_position();
        double get_mouse_x() const { return mouse_x > 0 ? mouse_x : 0.0 ; }
        double get_mouse_y() const { return mouse_y > 0 ? mouse_y : 0.0 ; }
        bool has_mouse_moved() const { return mouse_moved; }
        void reset_mouse_moved_flag() { mouse_moved = false; }


    private:
        GLFWwindow* window;
        EngineContext& engine_context;

        double mouse_x = 0.0;
        double mouse_y = 0.0;
        bool mouse_moved = false;

    };
}