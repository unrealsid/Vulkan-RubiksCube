// Vk_RubiksCube.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>

// VkTriangle.h
#ifndef VK_TRIANGLE_H
#define VK_TRIANGLE_H

#include <stdio.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>

#include "materials/ShaderObject.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

struct Init
{
    GLFWwindow* window;
    vkb::Instance instance;
    vkb::InstanceDispatchTable inst_disp;
    VkSurfaceKHR surface;
    vkb::Device device;
    vkb::DispatchTable disp;
    vkb::Swapchain swapchain;
};

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
};

GLFWwindow* create_window_glfw(const char* window_name = "", bool resize = true);
void destroy_window_glfw(GLFWwindow* window);
VkSurfaceKHR create_surface_glfw(VkInstance instance, GLFWwindow* window, VkAllocationCallbacks* allocator = nullptr);

int device_initialization(Init& init);
int create_swapchain(Init& init);
int get_queues(Init& init, RenderData& data);
std::vector<char> readFile(const std::string& filename);
VkShaderModule createShaderModule(Init& init, const std::vector<char>& code);
int create_graphics_pipeline(Init& init, RenderData& data);
int create_command_pool(Init& init, RenderData& data);
int create_command_buffers(Init& init, RenderData& data);
int create_sync_objects(Init& init, RenderData& data);
int recreate_swapchain(Init& init, RenderData& data);
int draw_frame(Init& init, RenderData& data);
void cleanup(Init& init, RenderData& data);

#endif // VK_TRIANGLE_H
