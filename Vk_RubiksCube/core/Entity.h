#pragma once

#include <memory>
#include "Transform.h"
#include "../structs/Vk_RenderData.h"
#include "../utils/MemoryUtils.h"
#include "string"

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
        Entity(uint32_t entity_id, EngineContext& engine_context, const std::string& entity_string_id);

        //Runs once when the game starts
        virtual void start();

        //Runs every frame
        virtual void update(double delta_time);
    
        Transform* get_transform() const { return transform.get(); }
        uint32_t get_entity_id() const { return entity_id; }
        std::string get_entity_string_id() const { return entity_string_id; }

        //We store transforms in buffers that are linked via an address -- I love Vulkan
        //Gets the sub allocated address from the global world matrix manager which is set from the engine
        [[nodiscard]] VkDeviceAddress get_transform_buffer_sub_address() const;

        [[nodiscard]] VkDeviceAddress get_object_id_buffer_address() const { return  object_id_buffer_address; }

        //Sent an updated world matrix data from cpu to GPU
        void update_transform_on_gpu(const glm::mat4& matrix_data) const;

    protected:
        vulkan::DeviceManager* device_manager;

        EngineContext& engine_context;

        std::unique_ptr<Transform> transform;

        virtual void initialize_transform() const;
        
    private:

        uint32_t entity_id;

        std::string entity_string_id;

        //Stores Object ID Address
        GPU_Buffer object_id_buffer;
        VkDeviceAddress object_id_buffer_address;

    };
}
