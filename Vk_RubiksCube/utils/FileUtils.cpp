#include "FileUtils.h"

void utils::FileUtils::loadShader(const std::string& filename, char*& code, size_t& size)
{
    std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);
    if (is.is_open())
    {
        size = is.tellg();
        is.seekg(0, std::ios::beg);
        code = new char[size];
        is.read(code, size);
        is.close();
        assert(size > 0);
    }
    else
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }
}

VkShaderModule utils::FileUtils::loadShader(const std::string& fileName, const EngineContext& engine_context)
{
    size_t shader_code_size{};
    char* shader_code{};
            
    loadShader(fileName, shader_code, shader_code_size);

    if(shader_code != nullptr)
    {
        VkShaderModule shader_module;
        VkShaderModuleCreateInfo module_create_info{};
        module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        module_create_info.codeSize = shader_code_size;
        module_create_info.pCode = reinterpret_cast<uint32_t*>(shader_code);

        engine_context.dispatch_table.createShaderModule(&module_create_info, nullptr, &shader_module);

        delete[] shader_code;

        return shader_module;
    }
    else
    {
        std::cerr << "Error: Could not open shader file \"" << fileName << "\"" << "\n";
        return VK_NULL_HANDLE;
    }
}
