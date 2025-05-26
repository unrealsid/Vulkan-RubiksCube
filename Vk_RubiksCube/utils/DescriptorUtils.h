#pragma once
#include <vector>
#include <vk_mem_alloc.h>

#include "VkBootstrapDispatch.h"
#include "../structs/Vk_Init.h"
#include "../structs/Vk_RenderData.h"
#include "../structs/Image.h"

struct Vk_Buffer;
struct SceneData;

namespace utils
{
    class DescriptorUtils
    {
    public:
        static void setupTextureDescriptors(const ::vkb::DispatchTable& disp, const std::vector<Image>& textures, VkDescriptorSetLayout& outDescriptorSetLayout, VkDescriptorSet&
                                            outDescriptorSet);

        static void mapUBO(const Init& init, SceneData& sceneDataUBO);
    };
}

namespace initializers
{
    inline VkDescriptorPoolSize descriptorPoolSize(
            VkDescriptorType type,
            uint32_t descriptorCount)
    {
        VkDescriptorPoolSize descriptorPoolSize {};
        descriptorPoolSize.type = type;
        descriptorPoolSize.descriptorCount = descriptorCount;
        return descriptorPoolSize;
    }

    inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
            VkDescriptorType type,
            VkShaderStageFlags stageFlags,
            uint32_t binding,
            uint32_t descriptorCount = 1)
    {
        VkDescriptorSetLayoutBinding setLayoutBinding {};
        setLayoutBinding.descriptorType = type;
        setLayoutBinding.stageFlags = stageFlags;
        setLayoutBinding.binding = binding;
        setLayoutBinding.descriptorCount = descriptorCount;
        return setLayoutBinding;
    }

    inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
            const std::vector<VkDescriptorSetLayoutBinding>& bindings)
    {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        return descriptorSetLayoutCreateInfo;
    }

    inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
            VkDescriptorPool descriptorPool,
            const VkDescriptorSetLayout* pSetLayouts,
            uint32_t descriptorSetCount)
    {
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
        descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
        return descriptorSetAllocateInfo;
    }

    inline VkWriteDescriptorSet writeDescriptorSet(
            VkDescriptorSet dstSet,
            VkDescriptorType type,
            uint32_t binding,
            VkDescriptorBufferInfo* bufferInfo,
            uint32_t descriptorCount = 1)
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

    inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
            const std::vector<VkDescriptorPoolSize>& poolSizes,
            uint32_t maxSets)
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        return descriptorPoolInfo;
    }
}
