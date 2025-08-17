#include "DrawableEntity.h"
#include "../vulkan/DeviceManager.h"

core::DrawableEntity::DrawableEntity(uint32_t entity_id, RenderData render_data, EngineContext& engine_context, const std::string& entity_string_id) :
                                                                                                                                                        Entity(entity_id, engine_context, entity_string_id),
                                                                                                                                                        render_data(std::move(render_data))
{
    
}

void core::DrawableEntity::update(double delta_time)
{
    Entity::update(delta_time);

    //Update model matrix and submit data to the gpu
    auto model_matrix = get_transform()->get_model_matrix();
    update_transform_on_gpu(model_matrix);
}

void core::DrawableEntity::initialize_transform() const
{
    transform->set_position(get_render_data().local_position);
    Entity::initialize_transform();
}
