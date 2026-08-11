#include "WindowManager.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "../core/Engine.h"
#include "../game_entities/GameManager.h"
#include "../structs/EngineContext.h"
#include "../rendering/Renderer.h"
#include "../vulkan/SwapchainManager.h"

namespace window
{
    WindowManager::WindowManager(EngineContext& engine_context) : last_mouse_x(0), last_mouse_y(0), mouse_delta_x(0),
                                                                  mouse_delta_y(0),
                                                                  window(nullptr),
                                                                  engine_context(engine_context)
    {
    }

    WindowManager::~WindowManager()
    {
        //Torn down here rather than by the engine so that the window always
        //outlives the Vulkan surface created from it.
        destroy_window();
    }

    SDL_Window* WindowManager::create_window(const char* windowName, bool resize)
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            std::cout << "SDL_Init failed: " << SDL_GetError() << "\n";
            return nullptr;
        }

        SDL_WindowFlags flags = SDL_WINDOW_VULKAN;
        if (resize)
        {
            flags |= SDL_WINDOW_RESIZABLE;
        }

        window = SDL_CreateWindow(windowName, window_width, window_height, flags);
        if (window == nullptr)
        {
            std::cout << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        }

        return window;
    }

    void WindowManager::destroy_window()
    {
        if (window != nullptr)
        {
            SDL_DestroyWindow(window);
            window = nullptr;
        }

        SDL_Quit();
    }

    VkSurfaceKHR WindowManager::create_surface(VkInstance instance, const VkAllocationCallbacks* allocator) const
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        if (!SDL_Vulkan_CreateSurface(window, instance, allocator, &surface))
        {
            std::cout << "SDL_Vulkan_CreateSurface failed: " << SDL_GetError() << "\n";
            surface = VK_NULL_HANDLE;
        }

        return surface;
    }

    void WindowManager::poll_events()
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                close_requested = true;
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                on_mouse_button(event.button);
                break;

            case SDL_EVENT_KEY_DOWN:
                on_key_interacted(event.key);
                break;

            default:
                break;
            }
        }
    }

    SDL_Window* WindowManager::get_window() const
    {
        return window;
    }

    void WindowManager::update_mouse_position()
    {
        float new_x, new_y;
        SDL_GetMouseState(&new_x, &new_y);

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

    void WindowManager::on_mouse_button(const SDL_MouseButtonEvent& event) const
    {
        //Rotate camera
        if (event.button == SDL_BUTTON_LEFT)
        {
            engine_context.renderer->should_update_camera = event.down;
        }
    }

    void WindowManager::on_key_interacted(const SDL_KeyboardEvent& event)
    {
        if (!event.repeat)
        {
            auto game_manager = dynamic_cast<GameManager*>(core::Engine::get_entity_by_tag("game_manager"));
            sequence += get_char_from_key(event.key);

            std::cout << sequence << std::endl;

            game_manager->execute_move_sequence(sequence);
        }

        sequence = "";
    }

    char WindowManager::get_char_from_key(int32_t key)
    {
        switch (key)
        {
            // Z-axis keys
        case SDLK_F: return 'F';
        case SDLK_B: return 'B';
        case SDLK_S: return 'S';

            // X-axis keys
        case SDLK_R: return 'R';
        case SDLK_L: return 'L';
        case SDLK_M: return 'M';

            // Y-axis keys
        case SDLK_U: return 'U';
        case SDLK_D: return 'D';
        case SDLK_E: return 'E';

        default: return '\0';
        }
    }
}
