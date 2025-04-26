#include "Vk_Utils.h"

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
