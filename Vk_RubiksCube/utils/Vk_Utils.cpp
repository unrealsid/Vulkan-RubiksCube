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
