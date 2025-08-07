#pragma once

#include "../core/DrawableEntity.h"

class RootEntity : public core::DrawableEntity
{
public:
    RootEntity(uint32_t entity_id, const RenderData& render_data, EngineContext& engine_context,
        const std::string& entity_string_id)
        : DrawableEntity(entity_id, render_data, engine_context, entity_string_id)
    {
    }

    virtual void update(double delta_time) override;

    virtual void initialize_transform() const override;
};
