#pragma once
#include <vector>
#include <vulkan.h>

namespace initializers
{
   VkDescriptorPoolSize descriptorPoolSize(
            VkDescriptorType type,
            uint32_t descriptorCount);

   VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
            VkDescriptorType type,
            VkShaderStageFlags stageFlags,
            uint32_t binding,
            uint32_t descriptorCount = 1);

   VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
            const std::vector<VkDescriptorSetLayoutBinding>& bindings);

   VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
            VkDescriptorPool descriptorPool,
            const VkDescriptorSetLayout* pSetLayouts,
            uint32_t descriptorSetCount);

   VkWriteDescriptorSet writeDescriptorSet(
            VkDescriptorSet dstSet,
            VkDescriptorType type,
            uint32_t binding,
            VkDescriptorBufferInfo* bufferInfo,
            uint32_t descriptorCount = 1);

   VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
            const std::vector<VkDescriptorPoolSize>& poolSizes,
            uint32_t maxSets);

   VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
                                    const VkDescriptorSetLayout* pSetLayouts,
                                    uint32_t setLayoutCount,
                                    const VkPushConstantRange* pPushConstantRanges = nullptr,
                                    uint32_t pushConstantRangeCount = 0);

   VkComputePipelineCreateInfo computePipelineCreateInfo(
                                        VkPipelineLayout layout, 
                                        VkPipelineCreateFlags flags = 0);
}
