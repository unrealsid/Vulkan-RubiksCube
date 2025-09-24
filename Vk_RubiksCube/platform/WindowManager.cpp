#include "WindowManager.h"
#include <iostream>
#include <GLFW/glfw3.h>

#include "../core/Engine.h"
#include "../game_entities/GameManager.h"
#include "../structs/EngineContext.h"
#include "../rendering/Renderer.h"
#include "../vulkan/SwapchainManager.h"

namespace window
{
    std::string WindowManager::sequence = "";

    WindowManager::WindowManager(EngineContext& engine_context) : last_mouse_x(0), last_mouse_y(0), mouse_delta_x(0),
                                                                  mouse_delta_y(0),
                                                                  window(nullptr),
                                                                  engine_context(engine_context)
    {
    }

    WindowManager::~WindowManager()
    {
    }

    GLFWwindow* WindowManager::create_window_glfw(const char* windowName, bool resize)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        if (!resize)
        {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        }

        window = glfwCreateWindow(window_width, window_height, windowName, nullptr, nullptr);
        register_callbacks();
        return window;
    }

    void WindowManager::destroy_window_glfw() const
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    VkSurfaceKHR WindowManager::create_surface_glfw(VkInstance instance, VkAllocationCallbacks* allocator) const
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

    void WindowManager::register_callbacks() const
    {
        glfwSetKeyCallback(window, on_key_interacted);
        glfwSetMouseButtonCallback(window, on_mouse_button);
    }

    GLFWwindow* WindowManager::get_window() const
    {
        return window;
    }

    void WindowManager::update_mouse_position()
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

    bool WindowManager::get_local_mouse_xy()
    {
        auto swapchain_extents = engine_context.swapchain_manager->get_swapchain().extent;

        if (mouse_x > 0 && mouse_x < swapchain_extents.width)
        {
            local_mouse_x = mouse_x;
        }

        if (mouse_y > 0 && mouse_y < swapchain_extents.height)
        {
            local_mouse_y = mouse_y;
        }


        //std::cout << "Local mouse x" << local_mouse_x << " local_mouse_y " << local_mouse_y << std::endl;

        return true;
    }

    void WindowManager::get_mouse_delta()
    {
        mouse_delta_x = local_mouse_x - last_mouse_x;
        mouse_delta_y = local_mouse_y - last_mouse_y;
    }

    void WindowManager::update_last_mouse_position()
    {
        last_mouse_x = local_mouse_x;
        last_mouse_y = local_mouse_y;
    }

    void WindowManager::on_mouse_button(GLFWwindow* window, int button, int action, int mods)
    {
        core::Engine& engine = core::Engine::get_instance();

        if(action == GLFW_PRESS)
        {
            //Rotate camera
            if(button == GLFW_MOUSE_BUTTON_LEFT)
            {
                engine.get_engine_context().renderer->should_update_camera = true;
            }
        }
        if(action == GLFW_RELEASE)
        {
            //Rotate camera
            if(button == GLFW_MOUSE_BUTTON_LEFT)
            {
                engine.get_engine_context().renderer->should_update_camera = false;
            }
        }
    }

    void WindowManager::on_key_interacted(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
        {
            auto game_manager =  dynamic_cast<GameManager*>(core::Engine::get_entity_by_tag("game_manager"));
            sequence += get_char_from_glfw_key(key);

            std::cout << sequence << std::endl;

            game_manager->execute_move_sequence(sequence);
        }

        sequence = "";
    }

    char WindowManager::get_char_from_glfw_key(int key)
    {
        switch (key)
        {
            // Z-axis keys
        case GLFW_KEY_F: return 'F';
        case GLFW_KEY_B: return 'B';
        case GLFW_KEY_S: return 'S';

            // X-axis keys
        case GLFW_KEY_R: return 'R';
        case GLFW_KEY_L: return 'L';
        case GLFW_KEY_M: return 'M';

            // Y-axis keys
        case GLFW_KEY_U: return 'U';
        case GLFW_KEY_D: return 'D';
        case GLFW_KEY_E: return 'E';

        default: return '\0';
        }
    }
}