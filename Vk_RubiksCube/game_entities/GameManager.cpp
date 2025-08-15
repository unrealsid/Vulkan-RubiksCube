#include "GameManager.h"

#include <iostream>

#include "../rendering/Renderer.h"
#include "../structs/EngineContext.h"
#include "../vulkan/SwapchainManager.h"
#include "../platform/WindowManager.h"
#include "../utils/GameUtils.h"
#include "../rendering/picking/ObjectPicking.h"
#include "../core/Engine.h"
#include "../utils/mouse_utils/ProjectionUtils.h"
#include "PointerEntity.h"

void GameManager::start()
{
    Entity::start();

    window_manager = engine_context.window_manager.get();
    object_picker = engine_context.renderer->get_object_picker();
    buffer =  object_picker->get_readback_buffer();

    parent_cubies_to_root();
    pointer_entity = dynamic_cast<PointerEntity*>(core::Engine::get_entity_by_tag("pointer"));
    cache_cubies();
}

void GameManager::update(double delta_time)
{
    Entity::update(delta_time);

    VkExtent2D swapchain_extents = engine_context.swapchain_manager->get_swapchain().extent;
    auto encoded_color = utils::GameUtils::get_pixel_color(engine_context,
        window_manager->local_mouse_x,
         window_manager->local_mouse_y, swapchain_extents, buffer);
    
    selected_object_id = utils::GameUtils::get_object_id_from_color(engine_context,
         window_manager->local_mouse_x,
         window_manager->local_mouse_y, swapchain_extents,
        buffer, encoded_color);

    if(Entity* entity =  core::Engine::get_drawable_entity_by_id(selected_object_id))
    {
        //Get location of pointer and get ID of cubie selected
        Transform* transform = entity->get_transform();
        
        std::cout << "Object id: << " << selected_object_id <<  " << Transform: " << *transform << std::endl;

        auto face_cubies = get_face_cubies();
        
        std::cout << "get_face_cubies() found " << face_cubies.size() << " cubies: ";
        for (uint32_t id : face_cubies)
        {
            std::cout << id << " ";
        }

        std::cout << std::endl;
        
        // std::cout << "Depth of selected point is: " << encoded_color.z << std::endl;
        // std::cout << "________________________________________________________________" << std::endl;
        //
        // Vk_SceneData scene_data = engine_context.renderer->get_scene_data();
        // selected_point = utils::ProjectionUtils::unproject_point(engine_context, encoded_color.z, scene_data.view, scene_data.projection);

        pointer_entity->get_transform()->set_position(selected_point);
    }
}

void GameManager::parent_cubies_to_root()
{
    auto& drawables = core::Engine::get_drawable_entities();
    auto root_drawable = core::Engine::get_entity_by_tag("root");
    
    for (const auto& cubie : drawables)
    {
        auto tag = cubie->get_entity_string_id();
        //Parent cubies to root
        if(tag.find("cubie_entity_") != std::string::npos)
        {
            cubie->get_transform()->parent = root_drawable->get_transform();
        }
    }
}

std::vector<uint32_t> GameManager::get_face_cubies() const
{
    Transform* selected_cubie_transform = get_cubie_transform(selected_object_id);
    glm::vec3 selected_position = selected_cubie_transform->get_world_position();

    std::vector<uint32_t> face_cubies;
    
    const float EPSILON = 0.001f;
    
    for (int cubie_id = 0; cubie_id < cubie_count; ++cubie_id)
    {
        auto cubie_transform = get_cubie_transform(cubie_id);
        glm::vec3 cubie_position{};

        if(cubie_transform)
        {
            cubie_position = cubie_transform->get_world_position();
        }
    }
    
    return face_cubies;
}

void GameManager::cache_cubies()
{
    auto& drawables = core::Engine::get_drawable_entities();
    
    for (const auto& cubie : drawables)
    {
        auto tag = cubie->get_entity_string_id();
        //Parent cubies to root
        if(tag.find("cubie_entity_") != std::string::npos)
        {
            cubies.push_back(cubie.get());
        }
    }

    cubie_count = cubies.size();
}

Transform* GameManager::get_cubie_transform(uint32_t cubie_id) const
{
    for (const auto& cubie : cubies)
    {
        if(cubie_id == cubie->get_entity_id())
        {
            return cubie->get_transform();
        }
    }
    
    return nullptr;
}

