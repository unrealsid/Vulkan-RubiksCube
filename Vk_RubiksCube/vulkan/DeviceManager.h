#pragma once
#include <vk_mem_alloc.h>

#include "SwapchainManager.h"
#include "VkBootstrap.h"

namespace window
{
    class WindowManager;
}

namespace vulkan
{
    class DeviceManager
    {
    public:
        DeviceManager();
        ~DeviceManager();
        
        bool device_init(window::WindowManager& windowManager);
        VkSurfaceKHR create_surface_GLFW(window::WindowManager& windowManager, VkAllocationCallbacks* allocator = nullptr);
        bool get_queues();

        bool create_command_pool();
        bool create_command_buffers();
        //bool create_graphics_pipeline();
        
    private:
        vkb::Instance instance;
        vkb::InstanceDispatchTable instance_dispatch_table;
        VkSurfaceKHR surface;
        vkb::Device device;
        vkb::PhysicalDevice physical_device;
        vkb::DispatchTable dispatch_table;

        SwapchainManager swapchain_manager;

        VkCommandPool command_pool;
        std::vector<VkCommandBuffer> command_buffers;

        VkQueue graphics_queue;
        VkQueue present_queue;

        VmaAllocator vmaAllocator;
        
    public:
        [[nodiscard]] vkb::Instance get_instance() const { return instance; }
        [[nodiscard]] vkb::InstanceDispatchTable getInstanceDispatchTable() const { return instance_dispatch_table; }
        [[nodiscard]] VkSurfaceKHR get_surface() const { return surface; }
        [[nodiscard]] vkb::Device get_device() const { return device; }
        [[nodiscard]] vkb::PhysicalDevice get_physical_device() const { return physical_device; }
        [[nodiscard]] vkb::DispatchTable get_dispatch_table() const { return dispatch_table; }
        [[nodiscard]] SwapchainManager get_swapchain_manager() const { return swapchain_manager; }
        [[nodiscard]] VkCommandPool get_command_pool() const { return command_pool; }
        [[nodiscard]] std::vector<VkCommandBuffer> get_command_buffers() const { return command_buffers; }
        [[nodiscard]] VkQueue get_graphics_queue() const { return graphics_queue; }
        [[nodiscard]] VkQueue get_present_queue() const { return present_queue; }
        [[nodiscard]] VmaAllocator get_allocator() const { return vmaAllocator; }

        void set_vma_allocator(VmaAllocator allocator) { vmaAllocator = allocator; }
    };
}

