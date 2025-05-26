#pragma once
#include <memory>
#include <vulkan_core.h>

struct GLFWwindow;

namespace window
{
    constexpr int windowWidth = 1280;
    constexpr int windowHeight = 720;
    
    class WindowManager
    {
    public:
        WindowManager();
        ~WindowManager();
        
        GLFWwindow* createWindowGLFW(const char* windowName = "", bool resize = true);
        void destroyWindowGLFW() const;
        VkSurfaceKHR createSurfaceGLFW(VkInstance instance, VkAllocationCallbacks* allocator = nullptr) const;
        bool shouldCloseWindow() const;

        GLFWwindow* getWindow() const;

    private:
        GLFWwindow* window;
    };
}