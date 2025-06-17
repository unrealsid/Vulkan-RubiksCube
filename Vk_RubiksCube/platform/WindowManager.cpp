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

GLFWwindow* window::WindowManager::create_window_glfw(const char* windowName, bool resize) 
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

void window::WindowManager::destroy_window_glfw() const
{
    glfwDestroyWindow(window);
    glfwTerminate();   
}

VkSurfaceKHR window::WindowManager::create_surface_glfw(VkInstance instance, VkAllocationCallbacks* allocator) const
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

GLFWwindow* window::WindowManager::get_window() const
{
    return window;
}

void window::WindowManager::update_mouse_position()
{
    double new_x, new_y;
    glfwGetCursorPos(window, &new_x, &new_y);
    
    // Check if mouse has moved
    if (new_x != mouse_x || new_y != mouse_y)
    {
        mouse_moved = true;
        mouse_x = new_x;
        mouse_y = new_y;
    }
}
