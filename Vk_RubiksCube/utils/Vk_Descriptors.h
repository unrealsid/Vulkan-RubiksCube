#pragma once
#include <vk_mem_alloc.h>

#include "VkBootstrapDispatch.h"
#include "../structs/Vk_Init.h"

struct Buffer;
struct SceneData;

namespace Vk_DescriptorUtils
{
    VkDescriptorSetLayout createDescriptorSetLayout(const vkb::DispatchTable& disp);
    
    VkDescriptorPool createDescriptorPool(const vkb::DispatchTable& disp, uint32_t maxSets);
    
    VkDescriptorSet allocateAndWriteDescriptorSet
    (
        const vkb::DispatchTable& disp,
        VkDescriptorPool descriptorPool,
        VkDescriptorSetLayout descriptorSetLayout,
        VkBuffer uniformBuffer,
        VkDeviceSize bufferSize);

    void createSceneBuffer(const Init& init, VkDeviceSize size, Buffer& buffer);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(const VkDescriptorSetLayout* pSetLayouts,
      uint32_t setLayoutCount, const VkPushConstantRange& pushConstantRange, uint32_t pushConstantCount);

    void mapUBO(const Init& init, SceneData& sceneDataUBO);
}
