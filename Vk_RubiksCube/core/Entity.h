#pragma once

#include <memory>
#include "Transform.h"
#include "../structs/Vk_RenderData.h"
#include "../utils/MemoryUtils.h"

struct EngineContext;

namespace vulkan
{
    class DeviceManager;
}

class Entity
{
public:
    Entity(uint32_t entity_id, RenderData render_data, EngineContext& engine_context);

    //Runs once when the game starts
    virtual void start();

    //Runs every frame
    virtual void update(double delta_time);
    
    RenderData get_render_data() const { return render_data; }
    Transform* get_transform() { return transform.get(); }
    uint32_t get_entity_id() const { return entity_id; }
    GPU_Buffer get_transform_buffer() const { return transform_buffer; }
    [[nodiscard]] VkDeviceAddress get_transform_buffer_address() const { return transform_buffer_address; }
    
private:

    uint32_t entity_id;
    RenderData render_data;

    //Stores transform address
    GPU_Buffer transform_buffer;
    VkDeviceAddress transform_buffer_address;

    std::unique_ptr<Transform> transform;

    EngineContext& engine_context;

    vulkan::DeviceManager* device_manager;
    
    //Init the buffer used to store object data
    //TODO: Unify this buffer and access via an ID
    void initial_transform_buffer();

    void initialize_transform() const;
};
