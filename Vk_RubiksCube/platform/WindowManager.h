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
        
        GLFWwindow* createWindowGLFW(const char* windowName = "", bool resize = true);
        void destroyWindowGLFW() const;
        VkSurfaceKHR createSurfaceGLFW(VkInstance instance, VkAllocationCallbacks* allocator = nullptr) const;

        bool refresh_frame() const;

        GLFWwindow* getWindow() const;

    private:
        GLFWwindow* window;
        EngineContext& engine_context;
    };
}