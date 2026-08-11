#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "DrawableEntity.h"
#include "Entity.h"
#include "../materials/Material.h"
#include "../rendering/camera/Camera.h"
#include "../structs/EngineContext.h"
#include "../structs/DrawBatch.h"
#include "../utils/ModelLoaderUtils.h"

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
    constexpr uint32_t MAX_OBJECTS = 500;
    
    class Engine
    {
    public:

        static Engine& get_instance();
    
        void init();
        void run();
        void cleanup();

        static std::vector<std::unique_ptr<DrawableEntity>>& get_drawable_entities() { return drawable_entities; }
        static std::vector<std::unique_ptr<Entity>>& get_entities() { return entities; }
        static Entity* get_entity_by_tag(const std::string& tag);

        static Entity* get_drawable_entity_by_id(uint32_t entity_id);
        EngineContext& get_engine_context() { return engine_context; };

    private:
        EngineContext engine_context;
        static std::vector<std::unique_ptr<DrawableEntity>> drawable_entities;
        static std::vector<std::unique_ptr<Entity>> entities;

        //Reference the orbit camera
        OrbitCamera orbit_camera;
        
        std::unordered_map<std::string, DrawBatch> draw_batches;

        Engine() = default;  
        ~Engine() = default; 
        Engine(const Engine&);
        Engine& operator=(const Engine&) = delete;

        //Loads the cubies
        void load_models();

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