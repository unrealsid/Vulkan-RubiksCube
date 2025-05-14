#include "Vk_Descriptors.h"

#include <iostream>
#include <vector>
#include <stdexcept>

#include "Vk_Utils.h"
#include "../structs/SceneData.h"

#include "VMA_MemoryUtils.h"

VkPhysicalDeviceDescriptorBufferFeaturesEXT Vk_DescriptorUtils::createPhysicalDeviceDescriptorBufferFeatures()
{
    VkPhysicalDeviceDescriptorBufferFeaturesEXT enabledDeviceDescriptorBufferFeaturesEXT{};
    enabledDeviceDescriptorBufferFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
    enabledDeviceDescriptorBufferFeaturesEXT.descriptorBuffer = VK_TRUE;

    return enabledDeviceDescriptorBufferFeaturesEXT;
}

void Vk_DescriptorUtils::prepareMVP_UBO(const Init& init, RenderData& data)
{
    createUniformBuffer(init, sizeof(SceneDataUBO), data.mvpUniformBufferInfo);
    SceneDataUBO sceneDataUBO;
    vkUtils::prepareUBO(sceneDataUBO);
    
    mapUBO(init, data.mvpUniformBufferInfo.allocation, sceneDataUBO);
}

void Vk_DescriptorUtils::setupDescriptors(const vkb::DispatchTable& disp, RenderData& data)
{
    VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
    descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayoutCI.bindingCount = 1;
    descriptorLayoutCI.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

    VkDescriptorSetLayoutBinding setLayoutBinding = {};

    setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setLayoutBinding.binding = 0;
    setLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    setLayoutBinding.descriptorCount = 1;

    descriptorLayoutCI.pBindings = &setLayoutBinding;
    disp.createDescriptorSetLayout(&descriptorLayoutCI, nullptr, &data.mvpDescriptorInfo.setLayout);
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

void Vk_DescriptorUtils::prepareDescriptorBuffer(Init& init, const vkb::InstanceDispatchTable& instancedDisp, const vkb::DispatchTable& disp, RenderData& data)
{
    // We need to get sizes and offsets for the descriptor layouts
    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties{};
    VkPhysicalDeviceProperties2KHR deviceProps2{};
    descriptorBufferProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;
    deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
    deviceProps2.pNext = &descriptorBufferProperties;
    instancedDisp.getPhysicalDeviceProperties2KHR(init.physicalDevice.physical_device, &deviceProps2);

    // Some devices have very low limits for the no. of max descriptor buffer bindings, so we need to check
    if (descriptorBufferProperties.maxResourceDescriptorBufferBindings < 2)
    {
        std::cerr << "This sample requires at least 2 descriptor bindings to run, the selected device only supports " + std::to_string(descriptorBufferProperties.maxResourceDescriptorBufferBindings);
    }

    disp.getDescriptorSetLayoutSizeEXT(data.mvpDescriptorInfo.setLayout, &data.mvpDescriptorInfo.layoutSize);
    disp.getDescriptorSetLayoutBindingOffsetEXT(data.mvpDescriptorInfo.setLayout, 0, &data.mvpDescriptorInfo.layoutOffset);

    // To copy resource descriptors to the correct place, we need to calculate aligned sizes
    data.mvpDescriptorInfo.layoutSize = alignedVkSize(data.mvpDescriptorInfo.layoutSize, descriptorBufferProperties.descriptorBufferOffsetAlignment);

    //Create the buffer that will store descriptor metadata
    vmaUtils::createBufferVMA(init.vmaAllocator, data.mvpDescriptorInfo.layoutSize,
        VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |  VMA_ALLOCATION_CREATE_MAPPED_BIT,
        data.mvpDescriptorInfo.bufferInfo.buffer, data.mvpDescriptorInfo.bufferInfo.allocation, data.mvpDescriptorInfo.bufferInfo.allocationInfo);

    //map that data
    //TODO: Change this
    vmaUtils::mapPersistenData(
        init.vmaAllocator,
        data.mvpDescriptorInfo.bufferInfo.allocation,
        data.mvpDescriptorInfo.bufferInfo.allocationInfo,
        data.mvpDescriptorInfo.bufferInfo.buffer, data.mvpDescriptorInfo.layoutSize);

    data.mvpDescriptorInfo.bufferDeviceAddress.deviceAddress = getBufferDeviceAddress(disp, data.mvpDescriptorInfo.bufferInfo.buffer);

    VkDescriptorGetInfoEXT descriptorInfo{};
    descriptorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;

    //Get the buffer address
    char* uniformDescriptorBufPtr = (char*)data.mvpDescriptorInfo.bufferInfo.allocationInfo.pMappedData;

    VkDescriptorAddressInfoEXT descriptorAddressInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT };
    descriptorAddressInfo.pNext = nullptr;
    descriptorAddressInfo.address = getBufferDeviceAddress(disp, data.mvpUniformBufferInfo.buffer);
    descriptorAddressInfo.range = data.mvpUniformBufferInfo.allocationInfo.size;
    descriptorAddressInfo.format = VK_FORMAT_UNDEFINED;

    //get MVP data
    descriptorInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorInfo.data.pCombinedImageSampler = nullptr;
    descriptorInfo.data.pUniformBuffer = &descriptorAddressInfo;
    disp.getDescriptorEXT(&descriptorInfo, descriptorBufferProperties.uniformBufferDescriptorSize, uniformDescriptorBufPtr);

    init.descriptorSetLayout = data.mvpDescriptorInfo.setLayout;
}

VkDeviceAddress Vk_DescriptorUtils::getBufferDeviceAddress(const vkb::DispatchTable disp, VkBuffer buffer)
{
    VkBufferDeviceAddressInfoEXT bufferDeviceAI{};
    bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAI.buffer = buffer;

    PFN_vkGetDeviceProcAddr fnGetDeviceProcAddr = (vkGetDeviceProcAddr);
    PFN_vkGetBufferDeviceAddressEXT fnGetBufferDeviceAddressEXT = reinterpret_cast<PFN_vkGetBufferDeviceAddressEXT>(vkGetDeviceProcAddr(disp.device, "vkGetBufferDeviceAddressKHR"));
    
    return fnGetBufferDeviceAddressEXT(disp.device, &bufferDeviceAI);
}

VkDeviceSize Vk_DescriptorUtils::alignedVkSize(VkDeviceSize value, VkDeviceSize alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

VkDescriptorPool Vk_DescriptorUtils::createDescriptorPool(const vkb::DispatchTable& disp, uint32_t maxSets)
{
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

void Vk_DescriptorUtils::createUniformBuffer(const Init& init, VkDeviceSize size, BufferInfo& bufferInfo)
{
    vmaUtils::createBufferVMA(init.vmaAllocator, size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,  
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, bufferInfo.buffer, bufferInfo.allocation, bufferInfo.allocationInfo);
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