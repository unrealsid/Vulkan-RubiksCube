#include "Vk_Utils.h"

#include <glm/fwd.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "../structs/SceneData.h"
#include "../structs/Vk_Init.h"

void vkUtils::SetVulkanObjectName(const vkb::DispatchTable& disp, uint64_t objectHandle,
                                  VkObjectType objectType, const std::string& name)
{
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = objectType;
        nameInfo.objectHandle = objectHandle;
        nameInfo.pObjectName = name.c_str();

        disp.setDebugUtilsObjectNameEXT(&nameInfo);
}

void vkUtils::fillSceneDataUBO(SceneData& sceneDataUBO, const glm::vec3& objectPosition,
    const glm::vec3& objectRotationAxis, float objectRotationAngleRadians, const glm::vec3& objectScale,
    const glm::vec3& cameraPosition, const glm::vec3& cameraTarget, const glm::vec3& cameraUp, float fieldOfViewRadians,
    float aspectRatio, float nearPlane, float farPlane)
{
    // --- Calculate the Model Matrix ---
    // Transforms vertices from model space to world space.
    // --- Calculate the Model Matrix ---
    sceneDataUBO.model = glm::mat4(1.0f);
    sceneDataUBO.model = glm::translate(sceneDataUBO.model, objectPosition);
    sceneDataUBO.model = glm::rotate(sceneDataUBO.model, objectRotationAngleRadians, objectRotationAxis);
    sceneDataUBO.model = glm::scale(sceneDataUBO.model, objectScale);

    // --- Calculate the View Matrix ---
    sceneDataUBO.view = glm::lookAt(cameraPosition, cameraTarget, cameraUp);

    // --- Calculate the Projection Matrix ---
    sceneDataUBO.proj = glm::perspective(fieldOfViewRadians, aspectRatio, nearPlane, farPlane);
    sceneDataUBO.proj[1][1] *= -1;
}

void vkUtils::prepareUBO(SceneData& sceneDataUBO)
{
    // Define your scene parameters
    glm::vec3 objPos = glm::vec3(2.0f, 0.5f, -3.0f);
    glm::vec3 objRotAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    float objRotAngle = glm::radians(45.0f);
    glm::vec3 objScale = glm::vec3(0.5f);

    glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 camTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float fov = glm::radians(45.0f);
    float aspect = 800.0f / 600.0f; // Example aspect ratio
    float nearZ = 0.1f;
    float farZ = 100.0f;

    // Call the function to fill the UBO data
    fillSceneDataUBO(
        sceneDataUBO,
        objPos, objRotAxis, objRotAngle, objScale,
        camPos, camTarget, camUp,
        fov, aspect, nearZ, farZ
    );
}
