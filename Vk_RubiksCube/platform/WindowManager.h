#pragma once
#include <memory>
#include <vulkan_core.h>

class GLFWwindow;

namespace window
{
    class WindowManager
    {
    public:
        WindowManager();
        ~WindowManager();

        constexpr int windowWidth = 1280;
        constexpr int windowHeight = 720;
    
        GLFWwindow* createWindowGLFW(const char* windowName = "", bool resize = true);
        void destroyWindowGLFW() const;
        VkSurfaceKHR createSurfaceGLFW(VkInstance instance, VkAllocationCallbacks* allocator = nullptr) const;

        GLFWwindow* getWindow() const;

    private:
        GLFWwindow* window;
    };
}