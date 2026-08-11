#include "GameManager.h"

#include <iostream>

#include "CubiesEntity.h"
#include "../core/Engine.h"
#include "../platform/WindowManager.h"
#include "../rendering/Renderer.h"
#include "../structs/EngineContext.h"
#include "../utils/mouse_utils/ProjectionUtils.h"

void GameManager::start()
{
    Entity::start();

    window_manager = engine_context.window_manager.get();

    cache_cubies();
    face_distance = calculate_face_distance();
    layer_threshold = face_distance * 0.5f;

    std::cout << "Cubies: " << cubie_count << ", face distance: " << face_distance
              << ", layer threshold: " << layer_threshold << std::endl;
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
}

// Appends a single move. Uppercase is clockwise, lowercase counter-clockwise.
// Unlike execute_move_sequence this leaves already-queued moves alone, so holding down
// a run of keys plays them back in the order they were pressed.
void GameManager::queue_move(char move_char)
{
    if (!isalpha(static_cast<unsigned char>(move_char)))
    {
        return;
    }

    move_queue.push(move_char);
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

    // If a face has no cubies, do nothing.
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

std::vector<CubiesEntity*> GameManager::get_face_cubies(char face) const
{
    //A cubie sits in the outer layer of an axis when it is more than half way out from the
    //centre. Testing against a mid-point rather than matching an exact distance keeps the
    //selection stable regardless of small asymmetries in the model or drift from past turns.
    std::vector<CubiesEntity*> face_cubies_entities;

    for (const auto& entity : cubies)
    {
        auto cubie = dynamic_cast<CubiesEntity*>(entity);
        if (!cubie) continue;

        const glm::vec3& pos = cubie->get_transform()->get_world_position();

        bool should_add = false;
        switch (face)
        {
            case 'F': should_add = pos.z >  layer_threshold; break;
            case 'B': should_add = pos.z < -layer_threshold; break;
            case 'R': should_add = pos.x >  layer_threshold; break;
            case 'L': should_add = pos.x < -layer_threshold; break;
            case 'U': should_add = pos.y >  layer_threshold; break;
            case 'D': should_add = pos.y < -layer_threshold; break;
            case 'M': should_add = std::abs(pos.x) <= layer_threshold; break;
            case 'E': should_add = std::abs(pos.y) <= layer_threshold; break;
            case 'S': should_add = std::abs(pos.z) <= layer_threshold; break;
            default: break;
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
    default: return glm::vec3(0.0f, 0.0f, 0.0f); 
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

