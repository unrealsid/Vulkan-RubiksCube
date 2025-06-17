#pragma once

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
        bool record_command_buffer(int32_t mouse_x, int32_t mouse_y) const;

        VkCommandBuffer get_command_buffer() const { return command_buffer; }

        VkFence get_object_picker_fence() const { return object_picker_fence; }

    private:
        EngineContext& engine_context;
        core::Engine* engine;

        DepthStencilImage depth_stencil_image;

        vulkan::DeviceManager* device_manager;
        vulkan::SwapchainManager* swapchain_manager;
        VkCommandPool command_pool;

        VkCommandBuffer command_buffer;

        Vk_Image object_id_image;
        GPU_Buffer object_id_buffer;

        void create_object_id_buffer();
        VkDeviceAddress object_id_buffer_address;

        VkFence object_picker_fence;

        void create_object_picking_material();

        //Stores the object picking material
        std::unique_ptr<material::Material> object_picker_material;

        //Creates the image attachment for storing Object IDs
        void create_image_attachment();
    };
}
