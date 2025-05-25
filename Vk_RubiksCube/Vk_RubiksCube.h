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

#include "structs/Vk_Init.h"
#include "structs/Vk_RenderData.h"

int create_graphics_pipeline(Init& init, RenderData& data);


int draw_frame(Init& init, RenderData& data);
void cleanup(Init& init, RenderData& data);

void loadModel(Init& init, RenderData& renderData);

#endif // VK_TRIANGLE_H
