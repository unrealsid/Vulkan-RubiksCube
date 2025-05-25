#pragma once
#include <vector>

#include "../materials/Material.h"

class MaterialManager;

namespace window
{
    class WindowManager;
}

namespace vulkan
{
    class DeviceManager;
}

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
    std::unique_ptr<MaterialManager> material_manager;
};
