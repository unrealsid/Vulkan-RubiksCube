#pragma once
#include <vector>

#include "../structs/Vk_SceneData.h"
#include <vulkan/vulkan.h>

#include "VkBootstrapDispatch.h"
#include "../structs/Vk_DepthStencilImage.h"

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
        Renderer(vulkan::DeviceManager* device_manager, vulkan::SwapchainManager* swapchain_manager);

        bool draw_frame();

        void init();

        VkBool32 get_supported_depth_stencil_format(VkPhysicalDevice physical_device, VkFormat* depth_stencil_format);

        void create_depth_stencil_image(VkExtent2D extents, VmaAllocator allocator, DepthStencilImage& depthImage);

        bool setup_scene_data();
        bool create_sync_objects();

        Vk_SceneData get_scene_data() const { return scene_data; }
        GPU_SceneData get_gpu_scene_data() const { return gpu_scene_data; }

        [[nodiscard]] std::vector<VkSemaphore> getAvailableSemaphores() const { return available_semaphores; }
        [[nodiscard]] std::vector<VkSemaphore> getFinishedSemaphores() const { return finished_semaphores; }
        [[nodiscard]] std::vector<VkFence> getInFlightFences() const { return in_flight_fences; }
        [[nodiscard]] std::vector<VkFence> getImageInFlight() const { return image_in_flight; }
        [[nodiscard]] size_t get_current_frame() const { return current_frame; }
    
    private:
        Vk_SceneData scene_data;
        GPU_SceneData gpu_scene_data;
        
        vulkan::DeviceManager* device_manager;
        vulkan::SwapchainManager* swapchain_manager;
        
        DepthStencilImage depth_stencil_image;

        std::vector<VkSemaphore> available_semaphores;
        std::vector<VkSemaphore> finished_semaphores;
        std::vector<VkFence> in_flight_fences;
        std::vector<VkFence> image_in_flight;
        size_t current_frame = 0;

        vkb::DispatchTable dispatch_table;
    };
}
