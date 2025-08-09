#pragma once

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

#include "../../structs/EngineContext.h"
#include "../../structs/GPU_Buffer.h"
#include "../../structs/Vk_DepthStencilImage.h" 
#include "../../structs/Vk_Image.h"

namespace material
{
    class Material;
}

namespace core
{
    class Engine;
}

namespace vulkan
{
    class DeviceManager;
    class SwapchainManager;
}

struct EngineContext;

namespace rendering
{
    class ObjectPicking
    {
    public:
        ObjectPicking(EngineContext& engine_context);

        bool first_submit_done;

        void init_picking();
        bool record_command_buffer(int32_t mouse_x, int32_t mouse_y);

        VkCommandBuffer get_command_buffer() const { return command_buffer; }

        VkFence get_object_picker_fence() const { return object_picker_fence; }

        GPU_Buffer get_readback_buffer() const { return readback_id_buffer; }

        GPU_Buffer get_face_normal_readback_buffer() const { return normal_readback_buffer;  }

        glm::vec3 get_selected_face_normal(const glm::mat4& model_transform) const;  

    private:
        EngineContext& engine_context;
        
        DepthStencilImage depth_stencil_image;

        vulkan::DeviceManager* device_manager;
        vulkan::SwapchainManager* swapchain_manager;
        VkCommandPool command_pool;

        VkCommandBuffer command_buffer;

        Vk_Image object_id_image;
        GPU_Buffer readback_id_buffer;

        GPU_Buffer normal_readback_buffer;

        VkFence object_picker_fence;

        //Stores the object picking material
        std::unique_ptr<material::Material> object_picker_material;

        void create_readback_id_buffer();

        //Creates a buffer for reading back the normal data for a face that is selected
        void create_normal_readback_buffer();
        VkDeviceAddress normal_readback_buffer_address;

        //Creates the material for object picking
        void create_object_picking_material();
        
        //Creates the image attachment for storing Object IDs
        void create_image_attachment();
    };
}
