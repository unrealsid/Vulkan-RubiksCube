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

    private:
        GLFWwindow* window;
        EngineContext& engine_context;
    };
}