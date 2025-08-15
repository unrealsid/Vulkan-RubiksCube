#pragma once
#include "../core/Entity.h"

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
          buffer{}, selected_object_id(-1)
    {
    }

    void start() override;

    void update(double delta_time) override;

    //setup a basic scene graph
    static void parent_cubies_to_root();

private:
    window::WindowManager* window_manager;
    rendering::ObjectPicking* object_picker;

    GPU_Buffer buffer;
    uint32_t selected_object_id;
    glm::vec3 selected_point;

    //How many cubies do we have? 
    uint32_t cubie_count;

    PointerEntity* pointer_entity;
    std::vector<core::DrawableEntity*> cubies;

    //Finds the selected cubie
    std::vector<uint32_t> get_face_cubies() const;

    //Cache cubies so they can be reused
    void cache_cubies();

    //Get selected cubie transform
    Transform* get_cubie_transform(uint32_t cubie_id) const;
};
