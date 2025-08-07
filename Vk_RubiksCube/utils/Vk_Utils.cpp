#include "Vk_Utils.h"

#include <glm/fwd.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "DescriptorUtils.h"
#include "DescriptorUtils.h"
#include "DescriptorUtils.h"
#include "DescriptorUtils.h"
#include "DescriptorUtils.h"
#include "DescriptorUtils.h"
#include "../platform/WindowManager.h"
#include "../structs/Vk_SceneData.h"

void utils::set_vulkan_object_Name(const vkb::DispatchTable& disp, uint64_t objectHandle,
                                  VkObjectType objectType, const std::string& name)
{
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = objectType;
        nameInfo.objectHandle = objectHandle;
        nameInfo.pObjectName = name.c_str();

        disp.setDebugUtilsObjectNameEXT(&nameInfo);
}

void utils::fill_scene_data_ubo(Vk_SceneData& sceneDataUBO,
    const glm::vec3& cameraPosition, const glm::vec3& cameraTarget, const glm::vec3& cameraUp, float fieldOfViewRadians,
    float aspectRatio, float nearPlane, float farPlane)
{
    // --- Calculate the View Matrix ---
    sceneDataUBO.view = glm::lookAt(cameraPosition, cameraTarget, cameraUp);

    // --- Calculate the Projection Matrix ---
    sceneDataUBO.projection = glm::perspective(fieldOfViewRadians, aspectRatio, nearPlane, farPlane);
    sceneDataUBO.projection[1][1] *= -1;
}

void utils::prepare_ubo(Vk_SceneData& sceneDataUBO)
{
    glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 camTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    float fov = glm::radians(45.0f);
    float aspect = static_cast<float>(window::window_width) / static_cast<float>(window::window_height); 
    float nearZ = 0.1f;
    float farZ = 100.0f;

    // Call the function to fill the UBO data
    fill_scene_data_ubo
    (
        sceneDataUBO,
        camPos, camTarget, camUp, fov,
        aspect, nearZ, farZ
    );
}
