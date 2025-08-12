#include "Initializers.h"

VkDescriptorPoolSize initializers::descriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount)
{
    VkDescriptorPoolSize descriptorPoolSize {};
    descriptorPoolSize.type = type;
    descriptorPoolSize.descriptorCount = descriptorCount;
    return descriptorPoolSize;
}

VkDescriptorSetLayoutBinding initializers::descriptorSetLayoutBinding(VkDescriptorType type,
    VkShaderStageFlags stageFlags, uint32_t binding, uint32_t descriptorCount)
{
    VkDescriptorSetLayoutBinding setLayoutBinding {};
    setLayoutBinding.descriptorType = type;
    setLayoutBinding.stageFlags = stageFlags;
    setLayoutBinding.binding = binding;
    setLayoutBinding.descriptorCount = descriptorCount;
    return setLayoutBinding;
}

VkDescriptorSetLayoutCreateInfo initializers::descriptorSetLayoutCreateInfo(
    const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.pBindings = bindings.data();
    descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    return descriptorSetLayoutCreateInfo;
}

VkDescriptorSetAllocateInfo initializers::descriptorSetAllocateInfo(VkDescriptorPool descriptorPool,
    const VkDescriptorSetLayout* pSetLayouts, uint32_t descriptorSetCount)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
    descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
    return descriptorSetAllocateInfo;
}

VkWriteDescriptorSet initializers::writeDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding,
    VkDescriptorBufferInfo* bufferInfo, uint32_t descriptorCount)
{
    VkWriteDescriptorSet writeDescriptorSet {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = dstSet;
    writeDescriptorSet.descriptorType = type;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.pBufferInfo = bufferInfo;
    writeDescriptorSet.descriptorCount = descriptorCount;
    return writeDescriptorSet;
}

VkDescriptorPoolCreateInfo initializers::descriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize>& poolSizes,
    uint32_t maxSets)
{
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    return descriptorPoolInfo;
}

VkPipelineLayoutCreateInfo initializers::pipelineLayoutCreateInfo(const VkDescriptorSetLayout* pSetLayouts,
    uint32_t setLayoutCount, const VkPushConstantRange* pPushConstantRanges, uint32_t pushConstantRangeCount)
{
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.setLayoutCount = setLayoutCount;
    info.pSetLayouts = pSetLayouts;
    info.pushConstantRangeCount = pushConstantRangeCount;
    info.pPushConstantRanges = pPushConstantRanges;
    return info;
}

VkComputePipelineCreateInfo initializers::computePipelineCreateInfo(VkPipelineLayout layout,
    VkPipelineCreateFlags flags)
{
    VkComputePipelineCreateInfo computePipelineCreateInfo {};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.layout = layout;
    computePipelineCreateInfo.flags = flags;
    return computePipelineCreateInfo;
}
