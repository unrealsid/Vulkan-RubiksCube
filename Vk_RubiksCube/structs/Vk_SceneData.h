#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "GPU_Buffer.h"

struct Vk_SceneData
{
    glm::mat4 view;
    glm::mat4 projection;
};

struct GPU_SceneBuffer
{
    GPU_Buffer scene_buffer;
    VkDeviceAddress scene_buffer_address;
};
