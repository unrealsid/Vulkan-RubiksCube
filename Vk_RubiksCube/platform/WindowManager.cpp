#include "WindowManager.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include "../structs/EngineContext.h"
#include "../rendering/Renderer.h"

window::WindowManager::WindowManager(EngineContext& engine_context): window(nullptr), engine_context(engine_context)
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

    window = glfwCreateWindow(window_width, window_height, windowName, nullptr, nullptr);
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

bool window::WindowManager::refresh_frame() const
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (bool result = engine_context.renderer->draw_frame(); !result)
        {
            std::cout << "failed to draw frame \n";
            return false;
        }
    }
    return true;
}

GLFWwindow* window::WindowManager::getWindow() const
{
    return window;
}
