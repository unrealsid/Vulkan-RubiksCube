#pragma once
#include "../core/Entity.h"

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

    void parent_cubies_to_root();

private:
    window::WindowManager* window_manager;
    rendering::ObjectPicking* object_picker;

    GPU_Buffer buffer;
    uint32_t selected_object_id;
    glm::vec3 selected_point;

    PointerEntity* pointer_entity;
};
