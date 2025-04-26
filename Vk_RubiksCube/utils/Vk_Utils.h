#pragma once
#include <string>
#include <vulkan_core.h>

#include "VkBootstrapDispatch.h"

struct Init;

namespace vkUtils
{
    void SetVulkanObjectName(const vkb::DispatchTable& disp, uint64_t objectHandle, VkObjectType objectType, const std::string& name);
}
