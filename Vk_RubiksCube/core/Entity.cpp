#include "Entity.h"
#include <memory>
#include "../utils/MemoryUtils.h"
#include "../vulkan/DeviceManager.h"
#include "../structs/EngineContext.h"
#include "../vulkan/DeviceManager.h"
#include <iostream>
#include "../utils/Vk_Utils.h"

core::Entity::Entity(uint32_t entity_id, EngineContext& engine_context, const std::string& entity_string_id): engine_context(engine_context),
                                                                                                              entity_id(entity_id),
                                                                                                              entity_string_id(entity_string_id),
                                                                                                              transform(std::make_unique<Transform>())
{
    device_manager = engine_context.device_manager.get();
}

void core::Entity::start()
{
    initialize_transform();
}

VkDeviceAddress core::Entity::get_transform_buffer_sub_address() const
{
    auto transform_buffer = engine_context.transform_manager->get_transforms_buffer();
    VkDeviceSize offset = entity_id * sizeof(glm::mat4);
    return transform_buffer.buffer_address + offset;
}

void core::Entity::update_transform_on_gpu(const glm::mat4& matrix_data) const
{
    auto transform_buffer = engine_context.transform_manager->get_transforms_buffer();
    auto model_transform = transform->get_model_matrix();

    auto offset = entity_id * sizeof(glm::mat4);
    utils::MemoryUtils::map_persistent_data(device_manager->get_allocator(), transform_buffer.allocation, transform_buffer.allocation_info, &model_transform, sizeof(glm::mat4), offset);
}

void core::Entity::initialize_transform() const
{
    glm::mat4 transform = get_transform()->get_model_matrix();
    update_transform_on_gpu(transform);
}

void core::Entity::update(double delta_time)
{
    
}
