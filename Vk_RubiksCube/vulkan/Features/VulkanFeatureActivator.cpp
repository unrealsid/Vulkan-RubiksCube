#include "VulkanFeatureActivator.h"

VkPhysicalDeviceDynamicRenderingFeaturesKHR vulkan::VulkanFeatureActivator::createDynamicRenderingFeatures()
{
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features{};
    dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamic_rendering_features.pNext = nullptr;
    dynamic_rendering_features.dynamicRendering = VK_TRUE;

    return dynamic_rendering_features;
}

VkPhysicalDeviceShaderObjectFeaturesEXT vulkan::VulkanFeatureActivator::createShaderObjectFeatures()
{
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features{};
    shader_object_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT;
    shader_object_features.pNext = nullptr;
    shader_object_features.shaderObject = VK_TRUE;

    return shader_object_features;
}

VkPhysicalDeviceBufferDeviceAddressFeatures vulkan::VulkanFeatureActivator::createPhysicaDeviceBufferAddress()
{
    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = 
    {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
        nullptr,
        VK_TRUE,
        VK_TRUE,
        VK_FALSE
    };

    return bufferDeviceAddressFeatures;
}

VkPhysicalDeviceDescriptorIndexingFeatures vulkan::VulkanFeatureActivator::createPhysicalDeviceDescriptorIndexingFeatures()
{
    VkPhysicalDeviceDescriptorIndexingFeatures features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
    features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features.runtimeDescriptorArray = VK_TRUE;
    features.descriptorBindingVariableDescriptorCount = VK_TRUE;

    return features;
}
