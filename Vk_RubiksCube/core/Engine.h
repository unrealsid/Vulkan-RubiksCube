#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "Entity.h"
#include "../materials/Material.h"
#include "../structs/EngineContext.h"
#include "../structs/DrawBatch.h"

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
        void run() const;
        void cleanup();

        static std::vector<std::unique_ptr<Entity>>& get_entities() { return entities; }

    private:
        EngineContext engine_context;
        static std::vector<std::unique_ptr<Entity>> entities;
        std::unordered_map<std::string, DrawBatch> draw_batches;

        void load_models();
        void organize_draw_batches();

        void update(double delta_timme) const;
        void render() const;
    };
}