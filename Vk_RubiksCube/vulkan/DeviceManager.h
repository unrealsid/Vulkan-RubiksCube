#pragma once
#include <vk_mem_alloc.h>

#include "SwapchainManager.h"
#include "VkBootstrap.h"

namespace window
{
    class WindowManager;
}

struct Init;

namespace vulkan
{
    class DeviceManager
    {
    public:
        constexpr int MAX_FRAMES_IN_FLIGHT = 2;
        
        DeviceManager();
        ~DeviceManager();
        
        bool deviceInit(window::WindowManager& windowManager);
        VkSurfaceKHR createSurfaceGLFW(window::WindowManager& windowManager, VkAllocationCallbacks* allocator = nullptr);
        bool getQueues();

        bool createCommandPool();
        bool createCommandBuffers();
        int createGraphicsPipeline();
        bool createSyncObjects();
        
    private:
        //calls loaded vulkan
        
        vkb::Instance instance;
        vkb::InstanceDispatchTable instance_dispatch_table;
        VkSurfaceKHR surface;
        vkb::Device device;
        vkb::PhysicalDevice physical_device;
        vkb::DispatchTable dispatch_table;

        SwapchainManager swapchain_manager;

        VkCommandPool command_pool;
        std::vector<VkCommandBuffer> command_buffers;

        std::vector<VkSemaphore> available_semaphores;
        std::vector<VkSemaphore> finished_semaphores;
        std::vector<VkFence> in_flight_fences;
        std::vector<VkFence> image_in_flight;
        size_t current_frame = 0;

        VkDescriptorSet descriptor_set;

        VkQueue graphics_queue;
        VkQueue present_queue;

        VmaAllocator vmaAllocator;
        
    public:
        [[nodiscard]] vkb::Instance getInstance() const { return instance; }
        [[nodiscard]] vkb::InstanceDispatchTable getInstanceDispatchTable() const { return instance_dispatch_table; }
        [[nodiscard]] VkSurfaceKHR getSurface() const { return surface; }
        [[nodiscard]] vkb::Device getDevice() const { return device; }
        [[nodiscard]] vkb::PhysicalDevice getPhysicalDevice() const { return physical_device; }
        [[nodiscard]] vkb::DispatchTable getDispatchTable() const { return dispatch_table; }
        [[nodiscard]] SwapchainManager getSwapchainManager() const { return swapchain_manager; }
        [[nodiscard]] VkCommandPool getCommandPool() const { return command_pool; }
        [[nodiscard]] std::vector<VkCommandBuffer> getCommandBuffers() const { return command_buffers; }
        [[nodiscard]] std::vector<VkSemaphore> getAvailableSemaphores() const { return available_semaphores; }
        [[nodiscard]] std::vector<VkSemaphore> getFinishedSemaphores() const { return finished_semaphores; }
        [[nodiscard]] std::vector<VkFence> getInFlightFences() const { return in_flight_fences; }
        [[nodiscard]] std::vector<VkFence> getImageInFlight() const { return image_in_flight; }
        [[nodiscard]] size_t getCurrentFrame() const { return current_frame; }
        [[nodiscard]] VkQueue getGraphicsQueue() const { return graphics_queue; }
        [[nodiscard]] VkQueue getPresentQueue() const { return present_queue; }
        [[nodiscard]] VkDescriptorSet getDescriptorSet() const { return descriptor_set; }
        [[nodiscard]] VmaAllocator& getAllocator() { return vmaAllocator; }
    };
}

