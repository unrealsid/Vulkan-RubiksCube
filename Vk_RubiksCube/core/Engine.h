#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "Entity.h"
#include "../enums/MouseDirection.h"
#include "../materials/Material.h"
#include "../structs/EngineContext.h"
#include "../structs/DrawBatch.h"
#include "../utils/mouse_utils/MouseTracker.h"

namespace renderer
{
    class Renderer;
}

namespace material
{
    class MaterialManager;
}


namespace window
{
    class WindowManager;
}

namespace vulkan
{
    class DeviceManager;
}

namespace core
{
    class Engine
    {
    public:
        Engine();
        ~Engine();
    
        void init();
        void run();
        void cleanup();

        static std::vector<std::unique_ptr<Entity>>& get_entities() { return entities; }

        static Entity* get_entity_by_id(uint32_t entity_id);

        static uint32_t get_selected_id() { return selected_object; }

    private:
        EngineContext engine_context;
        static std::vector<std::unique_ptr<Entity>> entities;
        std::unordered_map<std::string, DrawBatch> draw_batches;

        //Which ID is currently selected
        static uint32_t selected_object;

        static utils::MouseTracker mouse_tracker;
        static void get_mouse_direction(GLFWwindow* window);
        
        void load_models();
        void organize_draw_batches();

        void update(double delta_timme) const;
        void render() const;
    };
}