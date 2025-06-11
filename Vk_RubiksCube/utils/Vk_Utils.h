#pragma once
#include <string>
#include <vulkan_core.h>
#include <glm/vec3.hpp>

#include "VkBootstrapDispatch.h"
#include "../structs/EngineContext.h"
#include "../structs/Vk_MaterialData.h"

struct Vk_SceneData;
struct Init;

namespace utils
{
    void set_vulkan_object_Name(const vkb::DispatchTable& disp, uint64_t objectHandle, VkObjectType objectType, const std::string& name);

    void fill_scene_data_ubo(
        ::Vk_SceneData& sceneDataUBO,
        const glm::vec3& cameraPosition,
        const glm::vec3& cameraTarget,
        const glm::vec3& cameraUp,
        float fieldOfViewRadians,
        float aspectRatio,
        float nearPlane,
        float farPlane);

    void prepare_ubo( Vk_SceneData& sceneDataUBO);
}
