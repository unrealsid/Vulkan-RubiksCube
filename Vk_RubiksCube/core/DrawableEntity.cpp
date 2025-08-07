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
    utils::MemoryUtils::map_persistent_data(device_manager->get_allocator(), get_transform_buffer().allocation, get_transform_buffer().allocation_info, &model_matrix, sizeof(glm::mat4));
}

void core::DrawableEntity::initialize_transform() const
{
    transform->set_position(get_render_data().local_position);
    
}
