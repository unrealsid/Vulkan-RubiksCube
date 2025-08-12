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
    //buffer =  object_picker->get_readback_buffer();
    
    parent_cubies_to_root();
    pointer_entity = dynamic_cast<PointerEntity*>(core::Engine::get_entity_by_tag("pointer"));
}

void GameManager::update(double delta_time)
{
    Entity::update(delta_time);

    hit_detect = engine_context.renderer->get_hit_detect();
    buffer = hit_detect->get_readback_compute_buffer();
    
    if(auto data = static_cast<const glm::vec4*>(buffer.allocation_info.pMappedData))
    {
        std::cout << "(" << window_manager->local_mouse_x << ", " << window_manager->local_mouse_y << ")";
        std::cout << glm::to_string(*data) << std::endl;
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

