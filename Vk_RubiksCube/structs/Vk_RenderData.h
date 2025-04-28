#pragma once

#ifndef VK_RENDER_DATA_H
#define VK_RENDER_DATA_H

#include <memory>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan_core.h>

#include "Vk_DepthStencil_Image.h"

struct Vertex;
class ShaderObject;

struct RenderData
{
    VkQueue graphics_queue;
    VkQueue present_queue;

    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;

    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkSemaphore> available_semaphores;
    std::vector<VkSemaphore> finished_semaphore;
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> image_in_flight;
    size_t current_frame = 0;

    std::unique_ptr<ShaderObject> shader_object = nullptr;
    
    VkBuffer vertexBuffer;
    VmaAllocation vertexBufferAllocation;
    VkBuffer indexBuffer;
    VmaAllocation indexBufferAllocation;

    std::vector<uint32_t> outIndices;
    std::vector<Vertex> outVertices;

    VkDescriptorSet descriptorSet;

    DepthStencil_Image depthStencilImage;
};

#endif

