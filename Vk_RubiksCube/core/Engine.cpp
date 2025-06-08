#include "Engine.h"

#include <iostream>
#include <GLFW/glfw3.h>
#include "../platform/WindowManager.h"
#include "../utils/MemoryUtils.h"
#include "../utils/ModelLoaderUtils.h"
#include "../vulkan/DeviceManager.h"
#include "../materials/MaterialManager.h"
#include "../rendering/Renderer.h"
#include "../structs/EngineContext.h"
#include "../structs/DrawItem.h"


core::Engine::Engine()
{
}

core::Engine::~Engine()
{
}

void core::Engine::init()
{
    engine_context = EngineContext();
    
    //Init window and vulkan objects
    engine_context.window_manager = std::make_unique<window::WindowManager>();
    engine_context.device_manager = std::make_unique<vulkan::DeviceManager>();
    engine_context.swapchain_manager = std::make_unique<vulkan::SwapchainManager>();
    
    engine_context.window_manager->createWindowGLFW("Rubik's Cube", true);
    engine_context.device_manager->device_init(engine_context);

    engine_context.material_manager = std::make_unique<material::MaterialManager>(engine_context);

    auto swapchain_manager = engine_context.swapchain_manager.get();
    swapchain_manager->create_swapchain(engine_context);

    engine_context.device_manager->get_queues();
    utils::MemoryUtils::create_vma_allocator(*engine_context.device_manager);

    engine_context.renderer = std::make_unique<Renderer>(engine_context);
    engine_context.renderer->init();

    engine_context.renderer->create_command_pool();

    load_models();

    engine_context.renderer->create_command_buffers(); 
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

void core::Engine::load_models()
{
    std::vector<std::string> model_paths =
    {
        // "/models/rubiks_cube_texture/rubiksCubeTexture.obj",
        //"/models/rubiks_cube/rubiks_cube.obj",
        "/models/viper/viper.obj"
    };

    //Load all models 
    for (const auto& model_path : model_paths)
    {
        utils::ModelLoaderUtils model_utils;
        model_utils.load_model_from_obj(model_path, engine_context);
        auto indices = model_utils.get_material_index_ranges();

        //Create an entity for each loaded model
        std::unique_ptr<Entity> entity = std::make_unique<Entity>
        (
            RenderData
            {
                .vertex_buffer = model_utils.get_vertex_buffer(),
                .index_buffer = model_utils.get_index_buffer(),
                .vertices = model_utils.get_vertices(),
                .indices = model_utils.get_indices(),
            },
            indices
        );

        entities.push_back(std::move(entity));
    }

    //Once all materials are loaded, we can move them to the gpu
    engine_context.material_manager->init();
    organize_draw_batches();
}

void core::Engine::organize_draw_batches()
{
    auto material_manager = engine_context.material_manager.get();
    
    for (const auto& [name, material] : material_manager->get_materials())
    {
        DrawBatch batch;
        batch.shader_name = name;
        batch.material = material.get();
        draw_batches[name] = batch;
    }
    
    for (const auto& entity : entities)
    {
        const auto& render_data = entity->get_render_data();

        DrawItem item;
        item.vertex_buffer = render_data.vertex_buffer.buffer;
        item.index_buffer = render_data.index_buffer.buffer;
        item.index_count = render_data.indices.size();

        auto range = entity->get_material_index_range();

        for (const auto& [id, pair] : range)
        {
            std::string shader_name = material_manager->get_material_name_from_index(id);
            draw_batches[shader_name].items.push_back(item);    
        }
    }
}
