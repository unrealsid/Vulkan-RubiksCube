#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "../materials/Material.h"

namespace core
{
    class Renderer;
}

namespace material
{
    class MaterialManager;
}


namespace window
{
    class WindowManager;
}

namespace vulkan
{
    class DeviceManager;
}

namespace core
{
    class Engine
    {
    public:
        Engine();
        ~Engine();
    
        void init();
        void run();
        void cleanup();

    private:
        std::unique_ptr<window::WindowManager> window_manager;
        std::unique_ptr<vulkan::DeviceManager> device_manager;
        std::unique_ptr<material::MaterialManager> material_manager;
        std::unique_ptr<Renderer> renderer;

        void loadModels();
    };
}