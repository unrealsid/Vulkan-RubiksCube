#include "DrawableEntity.h"
#include "../vulkan/DeviceManager.h"

core::DrawableEntity::DrawableEntity(uint32_t entity_id, RenderData render_data, EngineContext& engine_context, const std::string& entity_string_id) :
                                                                                                                                                        Entity(entity_id, engine_context, entity_string_id),
                                                                                                                                                        render_data(std::move(render_data))
{
    
}

float i = 0.0;

void core::DrawableEntity::update(double delta_time)
{
    Entity::update(delta_time);

    //Update data on cpu and write to shared cpu/gpu memory
    i += 0.005f * delta_time;
    get_transform()->set_rotation(glm::vec3(90.0f, 90.0 * i, 0.0f));
    auto model_matrix = get_transform()->get_model_matrix();
    utils::MemoryUtils::map_persistent_data(device_manager->get_allocator(), get_transform_buffer().allocation, get_transform_buffer().allocation_info, &model_matrix, sizeof(glm::mat4));
}
