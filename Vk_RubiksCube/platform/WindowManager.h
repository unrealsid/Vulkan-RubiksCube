#pragma once
#include <functional>
#include <memory>
#include <vulkan_core.h>
#include <GLFW/glfw3.h>

namespace core
{
    class Engine;
}

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

        //Mouse coordinates clamped to the viewport
        int32_t local_mouse_x = 0;
        int32_t local_mouse_y = 0;

        int32_t last_mouse_x;
        int32_t last_mouse_y;
        
        int32_t mouse_delta_x;
        int32_t mouse_delta_y;

        GLFWwindow* create_window_glfw(const char* windowName = "", bool resize = true);
        void destroy_window_glfw() const;
        VkSurfaceKHR create_surface_glfw(VkInstance instance, VkAllocationCallbacks* allocator = nullptr) const;

        //Register window callbacks
        void register_callbacks() const;

        GLFWwindow* get_window() const;

        void update_mouse_position();
        double get_mouse_x() const { return mouse_x > 0 ? mouse_x : 0.0 ; }
        double get_mouse_y() const { return mouse_y > 0 ? mouse_y : 0.0 ; }

        bool get_local_mouse_xy(); 
        
        bool has_mouse_moved() const { return mouse_moved; }
        void reset_mouse_moved_flag() { mouse_moved = false; }
        void get_mouse_delta();
        void update_last_mouse_position();


    private:
        GLFWwindow* window;
        EngineContext& engine_context;

        double mouse_x = 0.0;
        double mouse_y = 0.0;
        bool mouse_moved = false;

        // Handle mouse button press/release
        static void on_mouse_button(GLFWwindow* window, int button, int action, int mods);

        // Handle mouse movement
        static void on_mouse_move(GLFWwindow* window, double xpos, double ypos);

        // Handle scroll wheel
        static void on_scroll(GLFWwindow* window, double xoffset, double yoffset);
    };
}