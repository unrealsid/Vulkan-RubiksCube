#pragma once
#include <vk_mem_alloc.h>

#include "SwapchainManager.h"
#include "VkBootstrap.h"

struct EngineContext;

namespace window
{
    class WindowManager;
}

namespace vulkan
{
    class DeviceManager
    {
    public:
        DeviceManager(EngineContext& engine_context);
        ~DeviceManager();
        
        bool device_init(EngineContext& engine_context);
        VkSurfaceKHR create_surface_GLFW(const EngineContext& engine_context, const VkAllocationCallbacks* allocator = nullptr) const;
        bool get_queues();
        
    private:
        vkb::Instance instance;
        VkSurfaceKHR surface;
        vkb::Device device;
        vkb::PhysicalDevice physical_device;
        EngineContext& engine_context;

        VkQueue graphics_queue;
        VkQueue present_queue;

        VmaAllocator vmaAllocator;
        
    public:
        [[nodiscard]] vkb::Instance get_instance() const { return instance; }
        [[nodiscard]] VkSurfaceKHR get_surface() const { return surface; }
        [[nodiscard]] vkb::Device get_device() const { return device; }
        [[nodiscard]] vkb::PhysicalDevice get_physical_device() const { return physical_device; }
        [[nodiscard]] VkQueue get_graphics_queue() const { return graphics_queue; }
        [[nodiscard]] VkQueue get_present_queue() const { return present_queue; }
        [[nodiscard]] VmaAllocator get_allocator() const { return vmaAllocator; }

        void set_vma_allocator(VmaAllocator allocator) { vmaAllocator = allocator; }
    };
}

