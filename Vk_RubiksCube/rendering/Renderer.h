#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "../structs/Vk_SceneData.h"
#include <vulkan/vulkan.h>

#include "VkBootstrapDispatch.h"
#include "../structs/Vk_DepthStencilImage.h"
#include "../structs/DrawBatch.h"

namespace rendering
{
    class ObjectPicking;
}

struct EngineContext;

namespace vulkan
{
    class SwapchainManager;
}

namespace vkb
{
    struct DispatchTable;
}

namespace vulkan
{
    class DeviceManager;
}

namespace core
{
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    
    class Renderer
    {
    public:
        Renderer(EngineContext& engine_context);

        void init();
        
        bool draw_frame();

        bool setup_scene_data();
        bool create_sync_objects();
        bool create_command_buffers();

        Vk_SceneData get_scene_data() const { return scene_data; }
        GPU_SceneBuffer get_gpu_scene_buffer() const { return gpu_scene_buffer; }

        [[nodiscard]] std::vector<VkSemaphore> get_available_semaphores() const { return available_semaphores; }
        [[nodiscard]] std::vector<VkSemaphore> getFinishedSemaphores() const { return finished_semaphores; }
        [[nodiscard]] std::vector<VkFence> getInFlightFences() const { return in_flight_fences; }
        [[nodiscard]] std::vector<VkFence> getImageInFlight() const { return image_in_flight; }
        [[nodiscard]] size_t get_current_frame() const { return current_frame; }
        
        [[nodiscard]] std::vector<VkCommandBuffer> get_command_buffers() const { return command_buffers; }
        [[nodiscard]] VkCommandPool get_command_pool() const { return command_pool; }
        [[nodiscard]] std::unordered_map<std::string, DrawBatch>& get_draw_batches() { return draw_batches; }
        [[nodiscard]] rendering::ObjectPicking* get_object_picker() const { return object_picker.get(); }
    
    private:
        Vk_SceneData scene_data;
        GPU_SceneBuffer gpu_scene_buffer;

        EngineContext& engine_context;
        
        DepthStencilImage depth_stencil_image;

        std::vector<VkSemaphore> available_semaphores;
        std::vector<VkSemaphore> finished_semaphores;
        VkSemaphore object_picker_done_semaphore;
        
        std::vector<VkFence> in_flight_fences;
        std::vector<VkFence> image_in_flight;
        
        size_t current_frame = 0;

        std::vector<VkCommandBuffer> command_buffers;
        VkCommandPool command_pool;

        vkb::DispatchTable dispatch_table;
        
        vulkan::DeviceManager* device_manager;
        vulkan::SwapchainManager* swapchain_manager;

        std::unordered_map<std::string, DrawBatch> draw_batches;

        std::unique_ptr<rendering::ObjectPicking> object_picker;

        void init_object_picker();

        void submit_object_picker_command_buffer() const;
    };
}
