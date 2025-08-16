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
#include "CubiesEntity.h"
#include "DynamicRootEntity.h"

void GameManager::start()
{
    Entity::start();

    window_manager = engine_context.window_manager.get();
    object_picker = engine_context.renderer->get_object_picker();
    buffer =  object_picker->get_readback_buffer();

    init_attach_cubies_to_root();
    //Get a reference to the dynamic root
    dynamic_root = dynamic_cast<DynamicRootEntity*>(core::Engine::get_entity_by_tag("dynamic_root"));
   
    pointer_entity = dynamic_cast<PointerEntity*>(core::Engine::get_entity_by_tag("pointer"));
    cache_cubies();
    face_distance = calculate_face_distance();
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
        // std::cout << "Depth of selected point is: " << encoded_color.z << std::endl;
        // std::cout << "________________________________________________________________" << std::endl;
        //
        // Vk_SceneData scene_data = engine_context.renderer->get_scene_data();
        // selected_point = utils::ProjectionUtils::unproject_point(engine_context, encoded_color.z, scene_data.view, scene_data.projection);

        pointer_entity->get_transform()->set_position(selected_point);
    }
}

void GameManager::init_attach_cubies_to_root()
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

void GameManager::attach_face_cubies_to_dynamic_root(DynamicRootEntity* dynamic_root_entity, std::vector<CubiesEntity*>& cubies_to_attach)
{
    for (const auto& cubie : cubies_to_attach)
    {
        //Parent cubies to new dynamic root
        auto* cubie_transform = cubie->get_transform();
        cubie_transform->parent = nullptr;
        cubie_transform->parent = dynamic_root_entity->get_transform();
    }

    dynamic_root_entity->can_rotate = true;
}

std::vector<CubiesEntity*> GameManager::get_face_cubies(char face) const
{
    std::vector<uint32_t> face_cubies;
        
    for (int i = 0; i < cubies.size(); ++i)
    {
        const glm::vec3& pos = cubies[i]->get_transform()->get_world_position();
        
        switch (face)
        {
            case 'F': // Front face (positive Z)
                {
                    float abs = std::abs(pos.z - face_distance);
                    if (abs < epsilon)
                    {
                        face_cubies.push_back(i);
                    }
                }
            break;
                
            case 'B': // Back face (negative Z)
                if (std::abs(pos.z + face_distance) < epsilon)
                {
                    face_cubies.push_back(i);
                }
                break;
                
            case 'R': // Right face (positive X)
                if (std::abs(pos.x - face_distance) < epsilon)
                {
                    face_cubies.push_back(i);
                }
                break;
                
            case 'L': // Left face (negative X)
                if (std::abs(pos.x + face_distance) < epsilon)
                {
                    face_cubies.push_back(i);
                }
                break;
                
            case 'U': // Up face (positive Y)
                if (std::abs(pos.y - face_distance) < epsilon)
                {
                    face_cubies.push_back(i);
                }
                break;
                
            case 'D': // Down face (negative Y)
                if (std::abs(pos.y + face_distance) < epsilon)
                {
                    face_cubies.push_back(i);
                }
                break;
                
            // Slice moves
            case 'M': // Middle slice (between L and R, turns like L)
                if (std::abs(pos.x) < epsilon)
                {
                    face_cubies.push_back(i);
                }
                break;
                
            case 'E': // Equatorial slice (between U and D, turns like D)
                if (std::abs(pos.y) < epsilon)
                {
                    face_cubies.push_back(i);
                }
                break;
                
            case 'S': // Standing slice (between F and B, turns like F)
                if (std::abs(pos.z) < epsilon)
                {
                    face_cubies.push_back(i);
                }
                break;
            default: ;
        }
    }

    std::vector<CubiesEntity*> cubies_to_rotate;
    for (const auto& cubie_id : face_cubies)
    {
        auto cubie = core::Engine::get_drawable_entity_by_id(cubie_id);
        cubies_to_rotate.push_back(dynamic_cast<CubiesEntity*>(cubie));
    }
    
    return cubies_to_rotate;
}

float GameManager::calculate_face_distance() const
{
    for (const auto& i : cubies)
    {
        float max_coord = 0.0f;
        for (const auto& cubie : cubies)
        {
            const glm::vec3& position = i->get_transform()->get_world_position();
            
            max_coord = std::max(max_coord, std::abs(position.x));
            max_coord = std::max(max_coord, std::abs(position.y));
            max_coord = std::max(max_coord, std::abs(position.z));
        }
        
        return max_coord;
    }
    
    return -1.0;
}

glm::vec3 GameManager::get_rotation_axis(char face)
{
    switch (face)
    {
    case 'F':
    case 'B':
    case 'S': // Standing slice rotates around Z-axis
        return glm::vec3(0.0f, 0.0f, 1.0f); // Z-axis
                
    case 'R':
    case 'L':
    case 'M': // Middle slice rotates around X-axis
        return glm::vec3(1.0f, 0.0f, 0.0f); // X-axis
                
    case 'U':
    case 'D':
    case 'E': // Equatorial slice rotates around Y-axis
        return glm::vec3(0.0f, 1.0f, 0.0f); // Y-axis
                
    default:
        return glm::vec3(0.0f, 1.0f, 0.0f);
    }
}

void GameManager::execute_move(const std::string& move_notation)
{
    
}

void GameManager::rotate_face(char face, bool clockwise)
{
    auto cubies_to_rotate = get_face_cubies(face);
    attach_face_cubies_to_dynamic_root(dynamic_root, cubies_to_rotate);
    auto rotation_axis = get_rotation_axis(face);
    dynamic_root->set_rotation_params(true, rotation_axis, 90);
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
            std::cout << "id: " << cubie->get_entity_id() << "\n";
            std::cout << "transform " << *cubie->get_transform();
            std ::cout << std::endl;
        }
    }

    cubie_count = cubies.size();
}

Transform* GameManager::get_cubie_transform(uint32_t cubie_id)
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

