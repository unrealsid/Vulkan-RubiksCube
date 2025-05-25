#include "Engine.h"

#include <iostream>
#include <GLFW/glfw3.h>

#include "../platform/WindowManager.h""
#include "../utils/MemoryUtils.h"
#include "../vulkan/DeviceManager.h"

Engine::Engine()
{
}

Engine::~Engine()
{
}

void Engine::init()
{
    //1. Init window and vulkan objects
    window_manager = std::make_unique<window::WindowManager>();
    window_manager->createWindowGLFW();
    
    device_manager = std::make_unique<vulkan::DeviceManager>();
    device_manager->deviceInit(*window_manager);
    device_manager->getSwapchainManager().createSwapchain(*device_manager);
    device_manager->getQueues();
    utils::MemoryUtils::createVmaAllocator(*device_manager);
    device_manager->createCommandPool();

    device_manager->createGraphicsPipeline();
    device_manager->createCommandBuffers();

    device_manager->createSyncObjects();
}

void Engine::run()
{
    while (!glfwWindowShouldClose(window_manager->getWindow()))
    {
        glfwPollEvents();
        int res = draw_frame(init, render_data);
        if (res != 0)
        {
            std::cout << "failed to draw frame \n";
            return -1;
        }
    }
}

void Engine::cleanup()
{
    
}
