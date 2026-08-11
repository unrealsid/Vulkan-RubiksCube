#pragma once
#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>

namespace core
{
    class Engine;
}

struct EngineContext;
struct SDL_Window;
struct SDL_KeyboardEvent;
struct SDL_MouseButtonEvent;

namespace window
{
    constexpr int window_width = 1280;
    constexpr int window_height = 720;

    class WindowManager
    {
    public:
        WindowManager(EngineContext& engine_context);
        ~WindowManager();

        WindowManager(const WindowManager&) = delete;
        WindowManager& operator=(const WindowManager&) = delete;

        //Mouse coordinates clamped to the viewport
        int32_t local_mouse_x = 0;
        int32_t local_mouse_y = 0;

        int32_t last_mouse_x;
        int32_t last_mouse_y;

        int32_t mouse_delta_x;
        int32_t mouse_delta_y;

        SDL_Window* create_window(const char* windowName = "", bool resize = true);
        void destroy_window();
        VkSurfaceKHR create_surface(VkInstance instance, const VkAllocationCallbacks* allocator = nullptr) const;

        //Drain the SDL event queue and dispatch to the handlers below
        void poll_events();
        bool should_close() const { return close_requested; }

        SDL_Window* get_window() const;

        void update_mouse_position();
        double get_mouse_x() const { return mouse_x > 0 ? mouse_x : 0.0 ; }
        double get_mouse_y() const { return mouse_y > 0 ? mouse_y : 0.0 ; }

        bool get_local_mouse_xy();

        bool has_mouse_moved() const { return mouse_moved; }
        void reset_mouse_moved_flag() { mouse_moved = false; }
        void get_mouse_delta();
        void update_last_mouse_position();


    private:
        SDL_Window* window;
        EngineContext& engine_context;

        double mouse_x = 0.0;
        double mouse_y = 0.0;
        bool mouse_moved = false;
        bool close_requested = false;

        // Handle mouse button press/release
        void on_mouse_button(const SDL_MouseButtonEvent& event) const;

        //Handle key inputs
        void on_key_interacted(const SDL_KeyboardEvent& event);

        static char get_char_from_key(int32_t key);
    };
}
