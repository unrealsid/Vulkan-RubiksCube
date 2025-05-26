#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Vk_Buffer.h"

struct SceneData
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

    Vk_Buffer sceneBuffer;
    VkDeviceAddress sceneBufferAddress;
};
