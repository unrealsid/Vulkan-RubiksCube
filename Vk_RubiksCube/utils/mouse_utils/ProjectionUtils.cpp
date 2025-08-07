#include "ProjectionUtils.h"
#include "../../platform/WindowManager.h"
#include "../../vulkan/SwapchainManager.h"

glm::vec3 utils::ProjectionUtils::unproject_point(const EngineContext& engine_context, float depth_value,
                                                  const glm::mat4& view_matrix, const glm::mat4& projection_matrix)
{
    auto window_manager = *engine_context.window_manager;
    auto extents = engine_context.swapchain_manager->get_swapchain().extent;
        
    // 1. Convert mouse coordinates to Normalized Device Coordinates (NDC)
    // NDC is in the range [-1, 1] for x and y.
    glm::vec4 screen_coords = glm::vec4(window_manager.local_mouse_x, extents.height - window_manager.local_mouse_y, depth_value, 1.0f);
    glm::vec4 ndc;
    ndc.x = (2.0f * screen_coords.x) / static_cast<float>(extents.width) - 1.0f;
    ndc.y = 1.0f - (2.0f * screen_coords.y) / static_cast<float>(extents.height) ;
    ndc.z = screen_coords.z * 2.0f - 1.0f; // Vulkan depth is [0, 1]. NDC z is typically [-1, 1]
    ndc.w = 1.0f;

    // 2. Combine inverse matrices
    glm::mat4 inverse_view_projection = glm::inverse(projection_matrix * view_matrix);

    // 3. Transform NDC to World Space
    glm::vec4 world_coords = inverse_view_projection * ndc;

    // 4. Perform Perspective Divide
    // The w component needs to be divided out to get the final 3D point.
    if (world_coords.w != 0.0f)
    {
        world_coords /= world_coords.w;
    }

    return {world_coords};
}
