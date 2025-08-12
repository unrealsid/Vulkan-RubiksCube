#pragma once
#include <vulkan_core.h>

struct Compute
{
    VkCommandPool command_pool{ VK_NULL_HANDLE };                     // Use a separate command pool (queue family may differ from the one used for graphics)
    VkCommandBuffer command_buffer{ VK_NULL_HANDLE };                 // Command buffer storing the dispatch commands and barriers
    VkSemaphore semaphore{ VK_NULL_HANDLE };                          // Execution dependency between compute & graphic submission
    VkDescriptorSetLayout descriptor_set_layout{ VK_NULL_HANDLE };    // Compute shader binding layout
    VkDescriptorSet descriptor_set{ VK_NULL_HANDLE };                 // Compute shader bindings
    VkPipelineLayout pipeline_layout{ VK_NULL_HANDLE };               // Layout of the compute pipeline
    VkPipeline pipeline;
};
