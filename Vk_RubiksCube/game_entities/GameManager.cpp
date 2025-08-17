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
    
    init_attach_cubies_to_root();
    dynamic_root = dynamic_cast<DynamicRootEntity*>(core::Engine::get_entity_by_tag("dynamic_root"));
    pointer_entity = dynamic_cast<PointerEntity*>(core::Engine::get_entity_by_tag("pointer"));
    
    cache_cubies();
    face_distance = calculate_face_distance();

    // Example of how to start a sequence
    //execute_move_sequence("FRU'L2B");
}

void GameManager::update(double delta_time)
{
    Entity::update(delta_time);

    // 1. If a face is currently rotating, check if it's finished.
    if (is_face_rotating)
    {
        bool all_finished = true;
        for (const auto& cubie : currently_rotating_cubies)
        {
            // We check the cubie's public 'can_rotate' flag.
            // If any cubie is still rotating, we can't be finished.
            if (cubie->is_rotating())
            {
                all_finished = false;
                break;
            }
        }

        // If all cubies have stopped, the move is complete.
        if (all_finished)
        {
            is_face_rotating = false;
            currently_rotating_cubies.clear();
            std::cout << "Move finished. Checking for next move in queue." << std::endl;
        }
    }
    // 2. If no face is rotating AND there are moves left in the queue, start the next one.
    else if (!move_queue.empty())
    {
        char move_char = move_queue.front();
        move_queue.pop();

        std::cout << "Executing next move: " << move_char << std::endl;

        // Uppercase moves are clockwise, lowercase are counter-clockwise
        bool is_clockwise = isupper(move_char);
        char face = toupper(move_char);

        rotate_face(face, is_clockwise);
    }

    buffer = object_picker->get_readback_buffer();

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
        Transform* transform = entity->get_transform();
        pointer_entity->get_transform()->set_position(selected_point);
    }
}

// This is the new public function you'll call to start a sequence of moves.
void GameManager::execute_move_sequence(const std::string& move_notation)
{
    // Clear any pending moves before starting a new sequence
    while(!move_queue.empty()) move_queue.pop();

    for (int i = 0; i < move_notation.length(); ++i)
    {
        char move = move_notation[i];

        // Ignore spaces or other non-alphabetic characters
        if (!isalpha(move)) continue;

        // Check for modifiers like ' (prime) or 2 (double turn)
        if (i + 1 < move_notation.length())
        {
            if (move_notation[i+1] == '\'')
            {
                move_queue.push(tolower(move)); // Use lowercase for counter-clockwise
                i++; // Skip the modifier character
                continue;
            }
            if (move_notation[i+1] == '2')
            {
                move_queue.push(toupper(move)); // Push the move twice for a 180-degree turn
                move_queue.push(toupper(move));
                i++; // Skip the modifier character
                continue;
            }
        }
        
        // If no modifier, it's a standard clockwise move
        move_queue.push(toupper(move));
    }
    std::cout << "Move sequence queued: " << move_notation << std::endl;
}

// This function is now responsible for initiating the rotation and setting our state flag.
void GameManager::rotate_face(char face, bool clockwise)
{
    // Safety check: don't start a new move if one is already in progress.
    if (is_face_rotating)
    {
        return;
    }

    auto cubies_to_rotate = get_face_cubies(face);

    // If a face has no cubies (e.g., invalid character), do nothing.
    if (cubies_to_rotate.empty())
    {
        return;
    }

    // Set the state to "rotating" and store the cubies for this move.
    is_face_rotating = true;
    currently_rotating_cubies = cubies_to_rotate;

    auto rotation_axis = get_rotation_axis(face);
    float angle = 90.0f;

    // In standard notation, turns for Back, Down, and Left faces are visually
    // clockwise but correspond to a negative rotation around the world axes.
    float direction = 1.0f;
    if (face == 'B' || face == 'D' || face == 'L')
    {
        direction = -1.0f;
    }

    // If the move is not clockwise (e.g., F'), invert the direction.
    if (!clockwise)
    {
        direction *= -1.0f;
    }

    float final_angle = angle * direction;

    for (auto cubie : currently_rotating_cubies)
    {
        // We assume CubiesEntity is castable from the Entity* in the cubies vector
        cubie->set_rotation_params(true, rotation_axis, final_angle);
    }
}

void GameManager::init_attach_cubies_to_root()
{
    auto& drawables = core::Engine::get_drawable_entities();
    auto root_drawable = core::Engine::get_entity_by_tag("root");
    
    for (const auto& cubie : drawables)
    {
        auto tag = cubie->get_entity_string_id();
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
        auto* cubie_transform = cubie->get_transform();
        cubie_transform->parent = nullptr;
        cubie_transform->parent = dynamic_root_entity->get_transform();
    }
    dynamic_root_entity->can_rotate = true;
}

std::vector<CubiesEntity*> GameManager::get_face_cubies(char face) const
{
    std::vector<CubiesEntity*> face_cubies_entities;
    for (const auto& entity : cubies)
    {
        auto cubie = dynamic_cast<CubiesEntity*>(entity);
        if (!cubie) continue;

        const glm::vec3& pos = cubie->get_transform()->get_world_position();
        
        bool should_add = false;
        switch (face)
        {
            case 'F': if (std::abs(pos.z - face_distance) < epsilon) should_add = true; break;
            case 'B': if (std::abs(pos.z + face_distance) < epsilon) should_add = true; break;
            case 'R': if (std::abs(pos.x - face_distance) < epsilon) should_add = true; break;
            case 'L': if (std::abs(pos.x + face_distance) < epsilon) should_add = true; break;
            case 'U': if (std::abs(pos.y - face_distance) < epsilon) should_add = true; break;
            case 'D': if (std::abs(pos.y + face_distance) < epsilon) should_add = true; break;
            case 'M': if (std::abs(pos.x) < epsilon) should_add = true; break;
            case 'E': if (std::abs(pos.y) < epsilon) should_add = true; break;
            case 'S': if (std::abs(pos.z) < epsilon) should_add = true; break;
        }
        if (should_add)
        {
            face_cubies_entities.push_back(cubie);
        }
    }
    return face_cubies_entities;
}

float GameManager::calculate_face_distance() const
{
    float max_coord = 0.0f;
    for (const auto& entity : cubies)
    {
        const glm::vec3& position = entity->get_transform()->get_world_position();
        max_coord = std::max({max_coord, std::abs(position.x), std::abs(position.y), std::abs(position.z)});
    }
    return max_coord;
}

glm::vec3 GameManager::get_rotation_axis(char face)
{
    switch (face)
    {
    case 'F': case 'B': case 'S': return glm::vec3(0.0f, 0.0f, 1.0f); // Z-axis
    case 'R': case 'L': case 'M': return glm::vec3(1.0f, 0.0f, 0.0f); // X-axis
    case 'U': case 'D': case 'E': return glm::vec3(0.0f, 1.0f, 0.0f); // Y-axis
    default: return glm::vec3(0.0f, 0.0f, 0.0f); // Should not happen
    }
}

void GameManager::cache_cubies()
{
    auto& drawables = core::Engine::get_drawable_entities();
    for (const auto& drawable : drawables)
    {
        if(drawable->get_entity_string_id().find("cubie_entity_") != std::string::npos)
        {
            cubies.push_back(drawable.get());
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

