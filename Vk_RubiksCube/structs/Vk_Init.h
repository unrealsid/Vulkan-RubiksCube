#pragma once

#ifndef VK_INIT_H
#define VK_INIT_H

#include <GLFW/glfw3.h>
#include "VkBootstrap.h"
#include <vma/vk_mem_alloc.h>

struct Init
{
    GLFWwindow* window;
    vkb::Instance instance;
    vkb::InstanceDispatchTable inst_disp;
    VkSurfaceKHR surface;
    vkb::Device device;
    vkb::PhysicalDevice physicalDevice;
    vkb::DispatchTable disp;
    
    vkb::Swapchain swapchain;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout { VK_NULL_HANDLE };

    VmaAllocator vmaAllocator;
};

#endif