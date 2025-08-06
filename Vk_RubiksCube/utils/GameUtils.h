#pragma once
#include <vulkan_core.h>
#include <glm/vec4.hpp>

#include "../structs/EngineContext.h"
#include "../structs/GPU_Buffer.h"

namespace utils
{
    class GameUtils
    {
    public:
        static glm::vec4 get_pixel_color(const EngineContext& engine_context, int32_t mouse_x, int32_t mouse_y, VkExtent2D swapchain_extent, GPU_Buffer& buffer);
        static uint32_t get_object_id_from_color(const EngineContext& engine_context, int32_t mouse_x, int32_t mouse_y, VkExtent2D swapchain_extent, GPU_Buffer& buffer);
    };
}
