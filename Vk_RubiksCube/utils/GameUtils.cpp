#include "GameUtils.h"
#include "DescriptorUtils.h"

glm::vec4 utils::GameUtils::get_pixel_color(const EngineContext& engine_context, int32_t mouse_x, int32_t mouse_y, VkExtent2D swapchain_extent, GPU_Buffer& buffer)
{
    //if(auto data = static_cast<const glm::vec4*>(buffer.allocation_info.pMappedData))
    //{
    //    uint32_t pixel_index = mouse_y * swapchain_extent.width + mouse_x;
    //    glm::vec4 pixel_color = data[pixel_index];

    //    // std::cout << "Color at (" << mouse_y << ", " << mouse_y << "): "
    //    //     << pixel_color.x << ", " << pixel_color.y << ", " << pixel_color.z << ", " << pixel_color.w << std::endl;

    //    return pixel_color;
    //}

    return glm::vec4(std::numeric_limits<float>::max());
}

uint32_t utils::GameUtils::get_object_id_from_color(const EngineContext& engine_context, int32_t mouse_x, int32_t mouse_y,
                                                    VkExtent2D swapchain_extent, GPU_Buffer& buffer, const glm::vec4& encoded_color)
{
    uint32_t id = static_cast<uint32_t>(floor(encoded_color.r * 56.0f));
    id = std::min(id, 55u);

    return id;
}
