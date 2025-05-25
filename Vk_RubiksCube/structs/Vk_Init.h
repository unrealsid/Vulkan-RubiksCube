#pragma once

#ifndef VK_INIT_H
#define VK_INIT_H

#include <memory>
#include <GLFW/glfw3.h>
#include "VkBootstrap.h"
#include <vma/vk_mem_alloc.h>


namespace window
{
    class WindowManager;
}

struct Init
{
    std::unique_ptr<window::WindowManager> window;
   
    

    VmaAllocator vmaAllocator;
};

#endif