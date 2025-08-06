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

namespace core
{
    class Entity
    {
    public:
        Entity(uint32_t entity_id, EngineContext& engine_context);

        //Runs once when the game starts
        virtual void start();

        //Runs every frame
        virtual void update(double delta_time);
    
        Transform* get_transform() const { return transform.get(); }
        uint32_t get_entity_id() const { return entity_id; }

        //We store transforms in buffers that are linked via an address -- I love Vulkan
        GPU_Buffer get_transform_buffer() const { return transform_buffer; }
        [[nodiscard]] VkDeviceAddress get_transform_buffer_address() const { return transform_buffer_address; }

        [[nodiscard]] VkDeviceAddress get_object_id_buffer_address() const { return  object_id_buffer_address; }

    protected:
        vulkan::DeviceManager* device_manager;

        EngineContext& engine_context;
        
    private:

        uint32_t entity_id;
        
        //Stores transform address
        GPU_Buffer transform_buffer;
        VkDeviceAddress transform_buffer_address;

        std::unique_ptr<Transform> transform;

        //Stores Object ID Address
        GPU_Buffer object_id_buffer;
        VkDeviceAddress object_id_buffer_address;
    
        //Init the buffer used to store object data
        void initialize_transform_buffer();

        void initialize_object_id_buffer();

        void initialize_transform() const;
    };
}
