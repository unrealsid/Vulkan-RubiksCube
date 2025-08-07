#pragma once

#include <vulkan/vulkan.h>

namespace vulkan
{
    class VulkanFeatureActivator
    {
    public:
        static VkPhysicalDeviceDynamicRenderingFeaturesKHR createDynamicRenderingFeatures();
        static VkPhysicalDeviceShaderObjectFeaturesEXT createShaderObjectFeatures();
        static VkPhysicalDeviceBufferDeviceAddressFeatures createPhysicaDeviceBufferAddress();
        static VkPhysicalDeviceDescriptorIndexingFeatures createPhysicalDeviceDescriptorIndexingFeatures();
    };
}