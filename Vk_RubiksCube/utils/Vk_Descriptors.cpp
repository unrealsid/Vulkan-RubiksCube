#include "Vk_Descriptors.h"
#include <vector>
#include <stdexcept>

#include "VMA_ImageUtils.h"
#include "../structs/SceneData.h"
#include "../structs/PushConstantBlock.h"
#include "../structs/Buffer.h"

#include "VMA_MemoryUtils.h"

VkPhysicalDeviceDescriptorIndexingFeatures Vk_DescriptorUtils::createPhysicalDeviceDescriptorIndexingFeatures()
{
    VkPhysicalDeviceDescriptorIndexingFeatures features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
    features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features.runtimeDescriptorArray = VK_TRUE;
    features.descriptorBindingVariableDescriptorCount = VK_TRUE;

    return features;
}

void Vk_DescriptorUtils::setupDescriptors(Init& init, RenderData& renderData)
{
    //Descriptor Pool
    std::vector<VkDescriptorPoolSize> poolSizes = { Vk_Initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(VMA_ImageUtils::textures.size())) };
    VkDescriptorPoolCreateInfo descriptorPoolInfo = Vk_Initializers::descriptorPoolCreateInfo(poolSizes, 1);

    VkDescriptorPool descriptorPool;
    init.disp.createDescriptorPool(&descriptorPoolInfo, nullptr, &descriptorPool);

    //Descriptor set layout
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
    setLayoutBindings.push_back(Vk_Initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, static_cast<uint32_t>(VMA_ImageUtils::textures.size())));

    VkDescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingFlags{};
    setLayoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    setLayoutBindingFlags.bindingCount = 1;

    // Enable variable descriptor count feature
    std::vector<VkDescriptorBindingFlags> descriptorBindingFlags =
    {
        0,
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
    };
    setLayoutBindingFlags.pBindingFlags = descriptorBindingFlags.data();

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = Vk_Initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
    descriptorSetLayoutCI.pNext = &setLayoutBindingFlags;

    //@TODO: Make global
    VkDescriptorSetLayout descriptorSetLayout;
    init.disp.createDescriptorSetLayout(&descriptorSetLayoutCI, nullptr, &descriptorSetLayout);

    init.descriptorSetLayout = descriptorSetLayout;
    
    // We need to provide the descriptor counts for bindings with variable counts using a new structure
    std::vector<uint32_t> variableDescriptorCounts =
    {
        static_cast<uint32_t>(VMA_ImageUtils::textures.size())
    };

    VkDescriptorSetVariableDescriptorCountAllocateInfo   variableDescriptorCountAllocInfo = {};
    variableDescriptorCountAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    variableDescriptorCountAllocInfo.descriptorSetCount = static_cast<uint32_t>(variableDescriptorCounts.size());
    variableDescriptorCountAllocInfo.pDescriptorCounts  = variableDescriptorCounts.data();

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = Vk_Initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
    descriptorSetAllocateInfo.pNext = &variableDescriptorCountAllocInfo;

    VkDescriptorSet descriptorSet;
    init.disp.allocateDescriptorSets(&descriptorSetAllocateInfo, &descriptorSet);

    renderData.descriptorSet = descriptorSet;

    std::vector<VkWriteDescriptorSet> writeDescriptorSets(1);

    std::vector<VkDescriptorImageInfo> textureDescriptors(VMA_ImageUtils::textures.size());
    for (size_t i = 0; i < VMA_ImageUtils::textures.size(); i++)
    {
        textureDescriptors[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        textureDescriptors[i].sampler = VMA_ImageUtils::textures[i].sampler;
        textureDescriptors[i].imageView = VMA_ImageUtils::textures[i].view;
    }

    writeDescriptorSets[0] = {};
    writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSets[0].dstBinding = 0;
    writeDescriptorSets[0].dstArrayElement = 0;
    writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSets[0].descriptorCount = static_cast<uint32_t>(VMA_ImageUtils::textures.size());
    writeDescriptorSets[0].pBufferInfo = 0;
    writeDescriptorSets[0].dstSet = renderData.descriptorSet;
    writeDescriptorSets[0].pImageInfo = textureDescriptors.data();

    init.disp.updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

VkDescriptorSetLayout Vk_DescriptorUtils::createDescriptorSetLayout(const vkb::DispatchTable& disp)
{
    VkDescriptorSetLayoutBinding uboBinding{};
    uboBinding.binding = 0; // The binding point in the shader
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1; // Number of descriptors in the binding (e.g., for arrays)
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Shader stages where this descriptor is used
    uboBinding.pImmutableSamplers = nullptr; // Optional sampler for image descriptors

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboBinding;

    VkDescriptorSetLayout descriptorSetLayout;
    if (disp.createDescriptorSetLayout(&layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    return descriptorSetLayout;
}

VkDescriptorPool Vk_DescriptorUtils::createDescriptorPool(const vkb::DispatchTable& disp, uint32_t maxSets) {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = maxSets; // Maximum number of descriptor sets that can be allocated with this type

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = maxSets; // Maximum number of descriptor sets that can be allocated from the pool
    poolInfo.flags = 0; // Optional flags, like VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT

    VkDescriptorPool descriptorPool;
    if (disp.createDescriptorPool(&poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    return descriptorPool;
}

VkDescriptorSet Vk_DescriptorUtils::allocateAndWriteDescriptorSet
(
    const vkb::DispatchTable& disp,
    VkDescriptorPool descriptorPool,
    VkDescriptorSetLayout descriptorSetLayout,
    VkBuffer uniformBuffer, VkDeviceSize bufferSize)
{

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1; // Number of descriptor sets to allocate
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VkDescriptorSet descriptorSet;
    if (disp.allocateDescriptorSets(&allocInfo, &descriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = bufferSize; // Size of the buffer region to bind

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet; // The descriptor set to update
    descriptorWrite.dstBinding = 0; // The binding point in the descriptor set
    descriptorWrite.dstArrayElement = 0; // The first element in a descriptor array to update
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1; // The number of descriptors to update
    descriptorWrite.pBufferInfo = &bufferInfo; // Information about the buffer resource

    disp.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

    return descriptorSet;
}

void Vk_DescriptorUtils::createBuffer(const Init& init, VkDeviceSize size, Buffer& buffer)
{
    vmaUtils::createBufferVMA(init.vmaAllocator, size,
                              VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,  
                              VMA_MEMORY_USAGE_AUTO,
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                              VMA_ALLOCATION_CREATE_MAPPED_BIT, buffer.buffer, buffer.allocation, buffer.allocationInfo);
}

VkPipelineLayoutCreateInfo Vk_DescriptorUtils::pipelineLayoutCreateInfo(const VkDescriptorSetLayout* pSetLayouts,
                                                                        uint32_t setLayoutCount, const VkPushConstantRange& pushConstantRange, uint32_t pushConstantCount)
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
    pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
    pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantCount;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    return pipelineLayoutCreateInfo;
}

void Vk_DescriptorUtils::mapUBO(const Init& init, SceneData& sceneDataUBO)
{
    void* mappedData;
    vmaMapMemory(init.vmaAllocator, sceneDataUBO.sceneBuffer.allocation, &mappedData);
    memcpy(mappedData, &sceneDataUBO, sizeof(sceneDataUBO));
    vmaUnmapMemory(init.vmaAllocator, sceneDataUBO.sceneBuffer.allocation);
}

// --- Example Usage (requires a Vulkan device and a created uniform buffer) ---
// This part is illustrative and requires a functional Vulkan context
/*
// Include necessary headers for the example main function if this was a standalone example
// #include <iostream> // For std::runtime_error

int main() {
    // Assume you have a VkDevice called 'device'
    // Assume you have a VkBuffer called 'uniformBuffer' with data for UniformBufferObject
    // Assume you have calculated the size of the buffer as 'uniformBufferSize'
    // VkDevice device = ...; // Your Vulkan device
    // VkBuffer uniformBuffer = ...; // Your uniform buffer
    // VkDeviceSize uniformBufferSize = sizeof(UniformBufferObject); // Size of your UBO

    try {
        // 1. Create the descriptor set layout
        // VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout(device);

        // 2. Create a descriptor pool
        // uint32_t maxSets = 1; // We only need one descriptor set in this simple example
        // VkDescriptorPool descriptorPool = createDescriptorPool(device, maxSets);

        // 3. Allocate and update the descriptor set
        // VkDescriptorSet descriptorSet = allocateAndWriteDescriptorSet(
        //    device,
        //    descriptorPool,
        //    descriptorSetLayout,
        //    uniformBuffer,
        //    uniformBufferSize);

        // Now you can use 'descriptorSetLayout' when creating your pipeline layout
        // and bind 'descriptorSet' in your command buffer before draw calls.

        // --- Cleanup (in your Vulkan cleanup process) ---
        // vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        // vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        // The uniformBuffer would also need to be destroyed and its memory freed.

    } catch (const std::runtime_error& e) {
        // Handle errors
        // std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
*/