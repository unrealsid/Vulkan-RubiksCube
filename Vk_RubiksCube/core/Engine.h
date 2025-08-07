#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "DrawableEntity.h"
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

        static std::vector<std::unique_ptr<DrawableEntity>>& get_drawable_entities() { return drawable_entities; }
        static std::vector<std::unique_ptr<Entity>>& get_entities() { return entities; }
        static Entity* get_entity_by_tag(const std::string& tag);

        static Entity* get_drawable_entity_by_id(uint32_t entity_id);

    private:
        EngineContext engine_context;
        static std::vector<std::unique_ptr<DrawableEntity>> drawable_entities;
        static std::vector<std::unique_ptr<Entity>> entities;
        
        std::unordered_map<std::string, DrawBatch> draw_batches;

        static utils::MouseTracker mouse_tracker;
        static void get_mouse_direction(GLFWwindow* window);
        
        void load_models();
        void load_root();
        void load_pointer();
        void load_entities();
        void organize_draw_batches();

        //Calls start event on entities
        static void exec_start();

        //Calls update on each entity
        static void update(double delta_timme);

        //Calls render on drawable entities
        void render() const;
    };
}