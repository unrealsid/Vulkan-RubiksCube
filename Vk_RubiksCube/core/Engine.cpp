#include "Engine.h"

#include <chrono>
#include <iostream>
#include <GLFW/glfw3.h>

#include "DrawableEntity.h"
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
utils::MouseTracker core::Engine::mouse_tracker(3.0);
uint32_t core::Engine::selected_object = -1;

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

    engine_context.renderer = std::make_unique<Renderer>(engine_context);
    engine_context.renderer->init();

    load_models();

    engine_context.renderer->create_command_buffers();
}

void core::Engine::get_mouse_direction(GLFWwindow* window)
{
    mouse_tracker.update_position(window);
    MouseDirection dir = mouse_tracker.get_direction();

    if (dir != MouseDirection::none)
    {
        //printf("Mouse moving: %s\n", utils::direction_to_string(dir));
            
        // Get actual movement values if needed
        double delta_x, delta_y;
        mouse_tracker.get_movement_delta(delta_x, delta_y);
        //printf("Delta: %.2f, %.2f\n", delta_x, delta_y);
    }
        
    mouse_tracker.commit_position();
}

void core::Engine::run()
{
    auto previous_time = std::chrono::high_resolution_clock::now();

    auto window_manager = engine_context.window_manager.get();
    auto window = engine_context.window_manager->get_window();
    auto object_picker =  engine_context.renderer->get_object_picker();
    GPU_Buffer buffer = object_picker->get_readback_buffer();
    
    while (!glfwWindowShouldClose(window))
    {
        window_manager->update_mouse_position();
        window_manager->get_local_mouse_xy();    
        
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = current_time - previous_time;
        double delta_time = elapsed.count();

        get_mouse_direction(window); 

        update(delta_time);
        render();

        //Store which object is currently selected.
        //TODO: Remove from loop and add to an empty Entity
        VkExtent2D swapchain_extents = engine_context.swapchain_manager->get_swapchain().extent;
        selected_object = utils::GameUtils::get_object_id_from_color(engine_context,
                                                                   window_manager->local_mouse_x,
                                                                   window_manager->local_mouse_y,
                                                                   swapchain_extents, buffer);

        std::cout << selected_object << std::endl;
        
        previous_time = current_time;

        glfwPollEvents();
   }
}

void core::Engine::cleanup()
{
    
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
        "/models/rubiks_cube/rubiks_cube.obj",
        //"/models/viper/viper.obj"
    };

    uint32_t entity_id = 0;
    
    //Load all models 
    for (const auto& model_path : model_paths)
    {
        utils::ModelLoaderUtils model_utils;
        model_utils.load_model_from_obj(model_path, engine_context);
        
        //For each shape in the obj file
        for (const auto& loaded_object : model_utils.get_loaded_objects())
        {
            ++entity_id;
            
            //Create an entity for each loaded shape
            std::unique_ptr<DrawableEntity> entity = std::make_unique<DrawableEntity>
            (
                entity_id,
                RenderData
                {
                    .vertex_buffer = loaded_object.vertex_buffer,
                    .index_buffer = loaded_object.index_buffer,
                    .vertices = loaded_object.vertices,
                    .indices = loaded_object.indices,
                    .material_index_ranges = loaded_object.material_index_ranges,
                },
                engine_context
            );
            
            drawable_entities.push_back(std::move(entity));
        }
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

void core::Engine::update(double delta_time) const
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
    
    // Record the object picking command buffer with new mouse position
   object_picker->record_command_buffer(window->local_mouse_x, window->local_mouse_y);
    
    if (bool result = engine_context.renderer->draw_frame(); !result)
    {
        //std::cout << "failed to draw frame \n";
    }
}
