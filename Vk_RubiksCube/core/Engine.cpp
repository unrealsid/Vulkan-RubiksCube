#include "Engine.h"

#include <chrono>
#include <iostream>
#include <GLFW/glfw3.h>

#include "DrawableEntity.h"
#include "../game_entities/CubiesEntity.h"
#include "../game_entities/DynamicRootEntity.h"
#include "../game_entities/GameManager.h"
#include "../game_entities/PointerEntity.h"
#include "../game_entities/RootEntity.h"
#include "../platform/WindowManager.h"
#include "../utils/MemoryUtils.h"
#include "../utils/ModelLoaderUtils.h"
#include "../vulkan/DeviceManager.h"
#include "../materials/MaterialManager.h"
#include "../rendering/Renderer.h"
#include "../structs/EngineContext.h"
#include "../structs/DrawItem.h"
#include "../rendering/picking/ObjectPicking.h"
#include "../utils/GameUtils.h"

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
    engine_context.device_manager = std::make_unique<vulkan::DeviceManager>();
    engine_context.swapchain_manager = std::make_unique<vulkan::SwapchainManager>();
    
    engine_context.window_manager->create_window_glfw("Rubik's Cube", true);
    engine_context.device_manager->device_init(engine_context);

    engine_context.material_manager = std::make_unique<material::MaterialManager>(engine_context);

    auto swapchain_manager = engine_context.swapchain_manager.get();
    swapchain_manager->create_swapchain(engine_context);

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
    load_root<DynamicRootEntity>(200, "dynamic_root");
    load_pointer();

    //Once all materials are loaded, we can move them to the gpu
    engine_context.material_manager->init();
    organize_draw_batches();
}

void core::Engine::load_pointer()
{
    std::string root_obj_path = "/models/mouse_pointer/mouse_pointer.obj";

    utils::ModelLoaderUtils model_utils;
    model_utils.load_model_from_obj(root_obj_path, engine_context);
    auto loaded_object = model_utils.get_loaded_objects()[0];

    //Create an entity for each loaded shape
    std::unique_ptr<DrawableEntity> entity = std::make_unique<PointerEntity>
    (
        300,
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
        "pointer"
    );
            
    drawable_entities.push_back(std::move(entity));
}

void core::Engine::load_entities()
{
    //Load Game manager
    std::unique_ptr<Entity> game_manager = std::make_unique<GameManager>(1000, engine_context, "game_manager");
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
    auto object_picker =  engine_context.renderer->get_object_picker();
    auto window = engine_context.window_manager.get();

    engine_context.renderer->update_camera(window->mouse_delta_x, window->mouse_delta_y);
    
    // Record the object picking command buffer with new mouse position
    //object_picker->record_command_buffer();
    
    if (bool result = engine_context.renderer->draw_frame(); !result)
    {
        //std::cout << "failed to draw frame \n";
    }
}