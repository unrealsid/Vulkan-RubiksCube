#pragma once
#include <string>
#include <vulkan_core.h>
#include <glm/vec3.hpp>

#include "VkBootstrapDispatch.h"

struct SceneData;
struct Init;

namespace vkUtils
{
    void SetVulkanObjectName(const vkb::DispatchTable& disp, uint64_t objectHandle, VkObjectType objectType, const std::string& name);

    void fillSceneDataUBO(
        SceneData& sceneDataUBO,
        const glm::vec3& objectPosition,
        const glm::vec3& objectRotationAxis,
        float objectRotationAngleRadians,
        const glm::vec3& objectScale,
        const glm::vec3& cameraPosition,
        const glm::vec3& cameraTarget,
        const glm::vec3& cameraUp,
        float fieldOfViewRadians,
        float aspectRatio,
        float nearPlane,
        float farPlane);

    void prepareUBO( SceneData& sceneDataUBO);
}
