#pragma once
#include <queue>

#include "../core/Entity.h"

class CubiesEntity;

namespace core
{
    class DrawableEntity;
}

class PointerEntity;

namespace rendering
{
    class ObjectPicking;
}

namespace window
{
    class WindowManager;
}

class GameManager : public core::Entity
{
public:
    GameManager(uint32_t entity_id, EngineContext& engine_context, const std::string& entity_string_id)
        : Entity(entity_id, engine_context, entity_string_id), window_manager(nullptr), object_picker(nullptr),
          buffer{}, selected_object_id(-1), cubie_count(0), face_distance(0.0f), layer_threshold(0.0f)
    {
    }

    void start() override;

    void update(double delta_time) override;

    //Get cubies that belong to a specific face
    std::vector<class CubiesEntity*> get_face_cubies(char face) const;

    // Find the maximum absolute coordinate value
    float calculate_face_distance() const;

    static glm::vec3 get_rotation_axis(char face);

    void execute_move_sequence(const std::string& move_notation);

    //Appends one move to the queue without discarding what is already pending
    void queue_move(char move_char);
    
   
private:
    window::WindowManager* window_manager;
    rendering::ObjectPicking* object_picker;

    GPU_Buffer buffer;
    uint32_t selected_object_id;
    glm::vec3 selected_point;

    //How many cubies do we have? 
    uint32_t cubie_count;

    //Distance from the centre to the outermost cubies, and the mid-point used to decide
    //which layer a cubie belongs to along a given axis.
    float face_distance;
    float layer_threshold;

    std::vector<core::DrawableEntity*> cubies;

    // A queue to hold the individual moves (e.g., 'F', 'u', 'R', 'R').
    // We'll use uppercase for clockwise and lowercase for counter-clockwise.
    std::queue<char> move_queue;

    // A flag to indicate if a face is currently animating.
    bool is_face_rotating = false;

    // A list of the specific cubies involved in the current rotation.
    // We'll use this to check when they have all finished.
    std::vector<CubiesEntity*> currently_rotating_cubies;
    
    //Cache cubies so they can be reused
    void cache_cubies();

    //Get selected cubie transform
    Transform* get_cubie_transform(uint32_t cubie_id);

    //Actually start rotating a face
    void rotate_face(char face, bool clockwise);
};
