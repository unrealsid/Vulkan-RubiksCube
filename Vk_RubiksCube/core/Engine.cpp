#include "Engine.h"

#include <chrono>
#include <iostream>
#include <GLFW/glfw3.h>

#include "DrawableEntity.h"
#include "../game_entities/CubiesEntity.h"
#include "../game_entities/GameManager.h"
#include "../game_entities/RootEntity.h"
#include "../platform/WindowManager.h"
#include "../utils/MemoryUtils.h"
#include "../utils/ModelLoaderUtils.h"
#include "../vulkan/DeviceManager.h"
#include "../materials/MaterialManager.h"
#include "../rendering/Renderer.h"
#include "../structs/EngineContext.h"
#include "../structs/DrawItem.h"

std::vector<std::unique_ptr<core::Entity>> core::Engine::entities;
std::vector<std::unique_ptr<core::DrawableEntity>> core::Engine::drawable_entities;

core::Engine& core::Engine::get_instance()
{
    static Engine instance;
    return instance;
}

void core::Engine::init()
{
    engine_context = EngineContext();
    
    //Init window and vulkan objects
    engine_context.window_manager = std::make_unique<window::WindowManager>(engine_context);
    engine_context.device_manager = std::make_unique<vulkan::DeviceManager>(engine_context);
    engine_context.swapchain_manager = std::make_unique<vulkan::SwapchainManager>(engine_context);
    
    engine_context.window_manager->create_window_glfw("Rubik's Cube", false);
    engine_context.device_manager->device_init(engine_context);

    engine_context.material_manager = std::make_unique<material::MaterialManager>(engine_context);

    auto swapchain_manager = engine_context.swapchain_manager.get();
    swapchain_manager->create_swapchain();

    engine_context.device_manager->get_queues();
    utils::MemoryUtils::create_vma_allocator(*engine_context.device_manager);

    //Initialize the transform manager
    engine_context.transform_manager = std::make_unique<TransformManager>(engine_context);
    
    engine_context.renderer = std::make_unique<Renderer>(engine_context);
    engine_context.renderer->init();
    orbit_camera = engine_context.renderer->get_camera();

    load_models();
    load_entities();

    engine_context.renderer->create_command_buffers();

    exec_start();
}

void core::Engine::run()
{
    auto previous_time = std::chrono::high_resolution_clock::now();

    auto window_manager = engine_context.window_manager.get();
    auto window = engine_context.window_manager->get_window();
    
    while (!glfwWindowShouldClose(window))
    {
        window_manager->update_mouse_position();
        window_manager->get_local_mouse_xy();
        window_manager->get_mouse_delta();
        
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = current_time - previous_time;
        double delta_time = elapsed.count();
        
        update(delta_time);
        render();
        
        previous_time = current_time;

        window_manager->update_last_mouse_position();
        
        glfwPollEvents();
   }
}

void core::Engine::cleanup()
{
    auto device_manager = engine_context.device_manager.get();
    auto swapchain_manager = engine_context.swapchain_manager.get();
    auto transform_manager = engine_context.transform_manager.get();
    auto material_manager = engine_context.material_manager.get();
    auto renderer = engine_context.renderer.get();

    auto transform_buffer = transform_manager->get_transforms_buffer();
    vmaDestroyBuffer(engine_context.device_manager->get_allocator(), transform_buffer.buffer, transform_buffer.allocation);

    for (auto& drawable : drawable_entities)
    {
        auto render_data = drawable->get_render_data();
        vmaDestroyBuffer(device_manager->get_allocator(), render_data.index_buffer.buffer, render_data.index_buffer.allocation);
        vmaDestroyBuffer(device_manager->get_allocator(), render_data.vertex_buffer.buffer, render_data.vertex_buffer.allocation);
    }
    
    engine_context.dispatch_table.destroyDescriptorSetLayout(material_manager->get_texture_descriptor_layout(), nullptr);
    engine_context.dispatch_table.destroyDescriptorPool(material_manager->get_texture_descriptor_pool(), nullptr);

    renderer->cleanup();
    swapchain_manager->cleanup();
}

core::Entity* core::Engine::get_entity_by_tag(const std::string& tag)
{
    for (const auto& element : drawable_entities)
    {
        if(element->get_entity_string_id() == tag)
        {
            return element.get();
        }
    }
    
    for (const auto& element : entities)
    {
        if(element->get_entity_string_id() == tag)
        {
            return element.get();
        }
    }

    return nullptr;
}

core::Entity* core::Engine::get_drawable_entity_by_id(uint32_t entity_id)
{
    for (const auto& entity : drawable_entities)
    {
        if(entity->get_entity_id() == entity_id)
        {
            return entity.get();
        }
    }

    return nullptr;
}

void core::Engine::load_models()
{
    std::vector<std::string> model_paths =
    {
        //"/models/rubiks_cube_texture/rubiksCubeTexture.obj",
        //"/models/plane/plane_simple.obj",
        //"/models/rubiks_cube/rubiks_cube.obj",
        "/models/rubiks_cube_newpivots/rubiks_cube_new_pivot.obj"
        //"/models/viper/viper.obj"
    };

    uint32_t entity_id = -1;
    
    //Load all models 
    for (const auto& model_path : model_paths)
    {
        utils::ModelLoaderUtils model_utils;
        model_utils.load_model_from_obj(model_path, engine_context);
        
        //For each shape in the obj file
        for (const auto& loaded_object : model_utils.get_loaded_objects())
        {
            //Create an entity for each loaded shape
            std::unique_ptr<DrawableEntity> entity = std::make_unique<CubiesEntity>
            (
                ++entity_id,
                RenderData
                {
                    .vertex_buffer = loaded_object.vertex_buffer,
                    .index_buffer = loaded_object.index_buffer,
                    .local_position = loaded_object.local_position,
                    .vertices = loaded_object.vertices,
                    .indices = loaded_object.indices,
                    .material_index_ranges = loaded_object.material_index_ranges,
                },
                engine_context,
                "cubie_entity_" + std::to_string(entity_id)
            );
            
            drawable_entities.push_back(std::move(entity));
        }
    }

    load_root<RootEntity>(100, "root");

    //Once all materials are loaded, we can move them to the gpu
    engine_context.material_manager->init();
    organize_draw_batches();
}

void core::Engine::load_entities()
{
    //Instantiate the Game manager
    std::unique_ptr<Entity> game_manager = std::make_unique<GameManager>(300, engine_context, "game_manager");
    entities.push_back(std::move(game_manager));
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
    
    for (const auto& entity : drawable_entities)
    {
        const auto& render_data = entity->get_render_data();

        auto range = render_data.material_index_ranges;

        for (const auto& [id, pair] : range)
        {
            DrawItem item;
            item.vertex_buffer = render_data.vertex_buffer.buffer;
            item.index_buffer = render_data.index_buffer.buffer;
            item.index_range = pair;
            item.entity = entity.get();

            //Second value MUST ALWAYS be less than first
            assert(pair.first < pair.second);

            item.index_count = pair.second - pair.first;
            
            std::string shader_name = material_manager->get_material_name_from_index(id);
            draw_batches[shader_name].items.push_back(item);    
        }
    }

    std::cout << "Draw batches: " << draw_batches.size() << "\n";
    auto renderer = engine_context.renderer.get();
    
    renderer->get_draw_batches() = std::move(draw_batches);
}

void core::Engine::exec_start()
{
    for (const auto& drawable_entity : drawable_entities)
    {
        drawable_entity->start();
    }

    for (const auto& entity : entities)
    {
        entity->start();
    }
}

void core::Engine::update(double delta_time)
{
    for (const auto& drawable_entity : drawable_entities)
    {
        drawable_entity->update(delta_time);
    }

    for (const auto& entity : entities)
    {
        entity->update(delta_time);
    }
}

void core::Engine::render() const
{
    auto window = engine_context.window_manager.get();

    engine_context.renderer->update_camera(window->mouse_delta_x, window->mouse_delta_y);
    
    if (bool result = engine_context.renderer->draw_frame(); !result)
    {
        std::cout << "failed to draw frame \n";
    }
}