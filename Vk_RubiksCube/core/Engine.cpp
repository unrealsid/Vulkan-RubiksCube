#include "Engine.h"

#include <iostream>
#include <GLFW/glfw3.h>

#include "Renderer.h"
#include "../platform/WindowManager.h"
#include "../utils/MemoryUtils.h"
#include "../utils/ModelUtils.h"
#include "../vulkan/DeviceManager.h"
#include "../materials/MaterialManager.h"

core::Engine::Engine()
{
}

core::Engine::~Engine()
{
}

void core::Engine::init()
{
    //1. Init window and vulkan objects
    window_manager = std::make_unique<window::WindowManager>();
    device_manager = std::make_unique<vulkan::DeviceManager>();
    
    window_manager->createWindowGLFW("Rubik's Cube", true);
    device_manager->device_init(*window_manager);

    material_manager = std::make_unique<material::MaterialManager>(device_manager.get());

    auto swapchain_manager = device_manager->get_swapchain_manager();
    swapchain_manager.create_swapchain(*device_manager);

    renderer = std::make_unique<Renderer>(device_manager.get(), &swapchain_manager);

    device_manager->get_queues();
    utils::MemoryUtils::create_vma_allocator(*device_manager);
    device_manager->create_command_pool();

    loadModels();

    //device_manager->createGraphicsPipeline();
    device_manager->create_command_buffers();
    renderer->init();
}

void core::Engine::run()
{
    // while (!window_manager->shouldCloseWindow())
    // {
    //     glfwPollEvents();
    //     int res = draw_frame(init, render_data);
    //     if (res != 0)
    //     {
    //         std::cout << "failed to draw frame \n";
    //         return -1;
    //     }
    // }
}

void core::Engine::cleanup()
{
    
}

void core::Engine::loadModels()
{
    std::vector<std::string> model_paths = { "/models/rubiks_cube_texture/rubiksCubeTexture.obj", "/models/rubiks_cube/rubiks_cube.obj" };
    
    for (const auto& model_path : model_paths)
    {
        utils::ModelUtils model_utils;
        model_utils.load_model_from_obj(model_path, *device_manager, *material_manager);
    }

    //Once all materials are loaded, we can move them to the gpu
    material_manager->init();
}
