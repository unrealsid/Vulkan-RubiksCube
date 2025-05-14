#pragma once
#include <vk_mem_alloc.h>

#include "VkBootstrapDispatch.h"
#include "../structs/Vk_Init.h"
#include "../structs/Vk_RenderData.h"

struct SceneDataUBO;

namespace Vk_DescriptorUtils
{

    VkPhysicalDeviceDescriptorBufferFeaturesEXT createPhysicalDeviceDescriptorBufferFeatures();

    //Copy data from host memory to device buffer
    void prepareMVP_UBO(const Init& init, RenderData& data);

    void setupDescriptors(const vkb::DispatchTable& disp, RenderData& data);
    
    VkDescriptorSetLayout createDescriptorSetLayout(const vkb::DispatchTable& disp);

    void prepareDescriptorBuffer(Init& init, const vkb::InstanceDispatchTable& instancedDisp, const vkb::DispatchTable& disp, RenderData& data);

    VkDeviceAddress getBufferDeviceAddress(vkb::DispatchTable disp, VkBuffer buffer);

    VkDeviceSize alignedVkSize(VkDeviceSize value, VkDeviceSize alignment);
    
    VkDescriptorPool createDescriptorPool(const vkb::DispatchTable& disp, uint32_t maxSets);
    
    VkDescriptorSet allocateAndWriteDescriptorSet
    (
        const vkb::DispatchTable& disp,
        VkDescriptorPool descriptorPool,
        VkDescriptorSetLayout descriptorSetLayout,
        VkBuffer uniformBuffer,
        VkDeviceSize bufferSize);

    void createUniformBuffer(const Init& init, VkDeviceSize size, BufferInfo& bufferInfo);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            const VkDescriptorSetLayout* pSetLayouts,
            uint32_t setLayoutCount = 1);

    void mapUBO(const Init& init, VmaAllocation uboAllocation, SceneDataUBO& sceneDataUBO);
}
