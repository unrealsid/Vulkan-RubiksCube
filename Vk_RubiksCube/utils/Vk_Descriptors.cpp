#include "Vk_Descriptors.h"
#include <vector>
#include <stdexcept>
#include "../structs/SceneData.h"

#include "VMA_MemoryUtils.h"

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

void Vk_DescriptorUtils::createUniformBuffer(const Init& init, VkDeviceSize size,
                                         VkBuffer& buffer, VmaAllocation& allocation)
{
    vmaUtils::createBufferVMA(init.vmaAllocator, size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,  
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, buffer, allocation);
}

VkPipelineLayoutCreateInfo Vk_DescriptorUtils::pipelineLayoutCreateInfo(const VkDescriptorSetLayout* pSetLayouts,
    uint32_t setLayoutCount)
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
    pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
    return pipelineLayoutCreateInfo;
}

void Vk_DescriptorUtils::mapUBO(const Init& init, VmaAllocation uboAllocation, SceneDataUBO& sceneDataUBO)
{
    void* mappedData;
    vmaMapMemory(init.vmaAllocator, uboAllocation, &mappedData);
    memcpy(mappedData, &sceneDataUBO, sizeof(sceneDataUBO));
    vmaUnmapMemory(init.vmaAllocator, uboAllocation);
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