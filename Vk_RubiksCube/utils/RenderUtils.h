#pragma once
#include <vk_mem_alloc.h>
#include <vulkan_core.h>

struct EngineContext;
struct DepthStencilImage;

namespace utils
{
    class RenderUtils
    {
    public:
        static bool create_command_pool(const EngineContext& engine_context, VkCommandPool& out_command_pool);

        static VkBool32 get_supported_depth_stencil_format(VkPhysicalDevice physical_device, VkFormat* depth_stencil_format);

        static bool create_depth_stencil_image(const EngineContext& engine_context, VkExtent2D extents, VmaAllocator allocator, DepthStencilImage& depth_image);
    };
}
