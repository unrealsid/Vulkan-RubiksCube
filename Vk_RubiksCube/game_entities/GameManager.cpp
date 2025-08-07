#include "GameManager.h"

#include <iostream>

#include "../rendering/Renderer.h"
#include "../structs/EngineContext.h"
#include "../vulkan/SwapchainManager.h"
#include "../platform/WindowManager.h"
#include "../utils/GameUtils.h"
#include "../rendering/picking/ObjectPicking.h"

void GameManager::start()
{
    Entity::start();

    window_manager = engine_context.window_manager.get();
    object_picker = engine_context.renderer->get_object_picker();
    buffer =  object_picker->get_readback_buffer();
}

void GameManager::update(double delta_time)
{
    Entity::update(delta_time);

    VkExtent2D swapchain_extents = engine_context.swapchain_manager->get_swapchain().extent;
    selected_object_id = utils::GameUtils::get_object_id_from_color(engine_context,
                                                               window_manager->local_mouse_x,
                                                               window_manager->local_mouse_y,
                                                               swapchain_extents, buffer);

    
    //Store which object is currently selected.
    //std::cout << selected_object_id << std::endl;
}

