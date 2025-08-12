#pragma once
#include <cassert>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <vulkan.h>

#include "../structs/EngineContext.h"

struct EngineContext;

namespace utils
{
    class FileUtils
    {
    public:
        static void loadShader(const std::string& filename, char* &code, size_t &size);

        static VkShaderModule loadShader(const std::string& fileName, const EngineContext& engine_context);
    };
}
