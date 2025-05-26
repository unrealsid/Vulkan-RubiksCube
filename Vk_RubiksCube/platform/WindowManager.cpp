#include "WindowManager.h"
#include <iostream>
#include <GLFW/glfw3.h>

window::WindowManager::WindowManager()
{
}

window::WindowManager::~WindowManager()
{
}

GLFWwindow* window::WindowManager::createWindowGLFW(const char* windowName, bool resize) 
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (!resize)
    {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    }

    window = glfwCreateWindow(windowWidth, windowHeight, windowName, nullptr, nullptr);
    return window;  
}

void window::WindowManager::destroyWindowGLFW() const
{
    glfwDestroyWindow(window);
    glfwTerminate();   
}

VkSurfaceKHR window::WindowManager::createSurfaceGLFW(VkInstance instance, VkAllocationCallbacks* allocator) const
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult err = glfwCreateWindowSurface(instance, window, allocator, &surface);
    if (err)
    {
        const char* error_msg;
        int ret = glfwGetError(&error_msg);
        
        if (ret != 0)
        {
            std::cout << ret << " ";
            if (error_msg != nullptr) std::cout << error_msg;
            std::cout << "\n";
        }
        surface = VK_NULL_HANDLE;
    }
    return surface;
}

bool window::WindowManager::shouldCloseWindow() const
{
    return glfwWindowShouldClose(window);  
}

GLFWwindow* window::WindowManager::getWindow() const
{
    return window;
}
