#include "Entity.h"
#include <memory>
#include "../utils/MemoryUtils.h"
#include "../vulkan/DeviceManager.h"
#include "../structs/EngineContext.h"
#include "../vulkan/DeviceManager.h"
#include <iostream>
#include "../utils/Vk_Utils.h"

Entity::Entity(uint32_t entity_id, RenderData render_data, EngineContext& engine_context): entity_id(entity_id),
    render_data(std::move(render_data)),
    transform(std::make_unique<Transform>()),
    engine_context(engine_context)
{
    device_manager = engine_context.device_manager.get();
    initialize_transform_buffer();
    initialize_transform();

    initialize_object_id_buffer();
}

void Entity::start()
{
    
}

void Entity::initialize_transform_buffer()
{
    utils::MemoryUtils::allocate_buffer_with_mapped_access(device_manager->get_allocator(), sizeof(glm::mat4), transform_buffer);
    transform_buffer_address = utils::MemoryUtils::get_buffer_device_address(engine_context.dispatch_table, transform_buffer.buffer);
}

void Entity::initialize_object_id_buffer()
{
    utils::MemoryUtils::allocate_buffer_with_mapped_access(device_manager->get_allocator(), sizeof(float), object_id_buffer);
    object_id_buffer_address = utils::MemoryUtils::get_buffer_device_address(engine_context.dispatch_table, object_id_buffer.buffer);
    utils::MemoryUtils::map_persistent_data(device_manager->get_allocator(), object_id_buffer.allocation, object_id_buffer.allocation_info, &entity_id, sizeof(float));

    utils::set_vulkan_object_Name(engine_context.dispatch_table, (uint64_t) object_id_buffer.buffer, VK_OBJECT_TYPE_BUFFER, "Object ID Buffer" + std::to_string(entity_id));
}

void Entity::initialize_transform() const
{
    transform->set_position(glm::vec3(0.0f, 0.0f, -5.0f));
    transform->set_rotation(glm::vec3(90.0f));
    transform->set_scale(glm::vec3(0.5f));
}

float i = 0.0;

void Entity::update(double delta_time)
{
    //Update data on cpu and write to shared cpu/gpu memory
    i += 0.005f * delta_time;
    transform->set_rotation(glm::vec3(90.0f, 90.0 * i, 0.0f));
    auto model_matrix = transform->get_model_matrix();
    utils::MemoryUtils::map_persistent_data(device_manager->get_allocator(), transform_buffer.allocation, transform_buffer.allocation_info, &model_matrix, sizeof(glm::mat4));
}
