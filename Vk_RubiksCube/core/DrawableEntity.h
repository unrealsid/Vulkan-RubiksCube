#pragma once
#include "Entity.h"

namespace core
{
    //Class used to render data
    class DrawableEntity : public Entity
    {
    public:
        DrawableEntity(uint32_t entity_id, RenderData render_data, EngineContext& engine_context);

        RenderData get_render_data() const { return render_data; }

        //Runs every frame
        virtual void update(double delta_time) override;

    private:
        //Stores data to render
        RenderData render_data;
    };
}