#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace utils
{
    //Compiles GLSL to SPIR-V at runtime via shaderc, so shaders no longer have to be
    //pre-compiled to .spv as a separate build step.
    class ShaderCompiler
    {
    public:
        //Compiles a GLSL source file into SPIR-V words.
        //#include directives are resolved relative to the shader's own directory and to
        //its "include" sub-directory.
        //Any failure (missing file, compile error) is reported on std::cerr and then asserts.
        static std::vector<uint32_t> compile_glsl_to_spirv(const std::string& shader_path, VkShaderStageFlagBits stage);

        //Directory holding the shader sources, resolved next to the running executable.
        static std::string get_shader_directory();
    };
}
