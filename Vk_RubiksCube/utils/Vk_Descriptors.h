#pragma once
#include <vk_mem_alloc.h>

#include "VkBootstrapDispatch.h"
#include "../structs/Vk_Init.h"

struct SceneDataUBO;

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

    void createUniformBuffer(const Init& init, VkDeviceSize size, VkBuffer& buffer, VmaAllocation& allocation);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            const VkDescriptorSetLayout* pSetLayouts,
            uint32_t setLayoutCount = 1);

    void mapUBO(const Init& init, VmaAllocation uboAllocation, SceneDataUBO& sceneDataUBO);
}
