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
    //Used for debugging vulkan objects in render doc or nvidia nsight
    void set_vulkan_object_Name(const vkb::DispatchTable& disp, uint64_t objectHandle, VkObjectType objectType, const std::string& name);
}
