#pragma once
#include "../core/DrawableEntity.h"

class PointerEntity : public core::DrawableEntity
{
public:
    PointerEntity(uint32_t entity_id, const RenderData& render_data, EngineContext& engine_context,
        const std::string& entity_string_id)
        : DrawableEntity(entity_id, render_data, engine_context, entity_string_id)
    {
    }

    void update(double delta_time) override;
    void initialize_transform() const override;
};
