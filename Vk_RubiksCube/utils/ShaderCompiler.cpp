#include "ShaderCompiler.h"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include <SDL3/SDL_filesystem.h>
#include <shaderc/shaderc.hpp>

namespace
{
    //Reports a fatal shader problem and stops. assert() is compiled out under NDEBUG, so the
    //abort keeps release builds from carrying on with a shader that never compiled.
    [[noreturn]] void fail(const std::string& message)
    {
        std::cerr << "[ShaderCompiler] " << message << std::endl;
        assert(false && "shader compilation failed");
        std::abort();
    }

    bool read_file(const std::string& path, std::string& out_contents)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }

        std::ostringstream contents;
        contents << file.rdbuf();
        out_contents = contents.str();
        return true;
    }

    //#include paths in the shaders are written both as "include/foo.glsl" and "/include/foo.glsl".
    //Dropping any leading separator lets both forms resolve against the same search directories.
    std::string strip_leading_separators(const std::string& path)
    {
        size_t first = 0;
        while (first < path.size() && (path[first] == '/' || path[first] == '\\'))
        {
            ++first;
        }
        return path.substr(first);
    }

    shaderc_shader_kind to_shaderc_kind(VkShaderStageFlagBits stage)
    {
        switch (stage)
        {
        case VK_SHADER_STAGE_VERTEX_BIT:                  return shaderc_vertex_shader;
        case VK_SHADER_STAGE_FRAGMENT_BIT:                return shaderc_fragment_shader;
        case VK_SHADER_STAGE_COMPUTE_BIT:                 return shaderc_compute_shader;
        case VK_SHADER_STAGE_GEOMETRY_BIT:                return shaderc_geometry_shader;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:    return shaderc_tess_control_shader;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return shaderc_tess_evaluation_shader;
        default:
            fail("Unsupported shader stage: " + std::to_string(static_cast<uint32_t>(stage)));
        }
    }

    //Resolves #include directives against the including file's directory and the
    //shader directory's "include" folder.
    class FileIncluder final : public shaderc::CompileOptions::IncluderInterface
    {
    public:
        explicit FileIncluder(std::vector<std::string> search_paths) : search_paths(std::move(search_paths))
        {
        }

        shaderc_include_result* GetInclude(const char* requested_source,
                                           shaderc_include_type,
                                           const char* requesting_source,
                                           size_t) override
        {
            auto payload = std::make_unique<Payload>();

            const std::string relative = strip_leading_separators(requested_source);

            //The directory of whoever issued the #include is searched first.
            std::vector<std::string> candidates;
            const std::filesystem::path requester(requesting_source);
            if (requester.has_parent_path())
            {
                candidates.push_back((requester.parent_path() / relative).string());
            }
            for (const auto& search_path : search_paths)
            {
                candidates.push_back((std::filesystem::path(search_path) / relative).string());
            }

            for (const auto& candidate : candidates)
            {
                if (read_file(candidate, payload->content))
                {
                    payload->path = candidate;
                    payload->result.source_name = payload->path.c_str();
                    payload->result.source_name_length = payload->path.size();
                    payload->result.content = payload->content.c_str();
                    payload->result.content_length = payload->content.size();
                    payload->result.user_data = payload.get();
                    return &payload.release()->result;
                }
            }

            //A zero-length source name is how shaderc is told the include failed; the
            //message in content is surfaced as part of the compile error.
            payload->content = "Could not resolve #include \"" + std::string(requested_source) + "\"";
            payload->result.source_name = "";
            payload->result.source_name_length = 0;
            payload->result.content = payload->content.c_str();
            payload->result.content_length = payload->content.size();
            payload->result.user_data = payload.get();
            return &payload.release()->result;
        }

        void ReleaseInclude(shaderc_include_result* data) override
        {
            delete static_cast<Payload*>(data->user_data);
        }

    private:
        //Backing storage for one include; shaderc only borrows the pointers, so the
        //strings have to outlive the result until ReleaseInclude runs.
        struct Payload
        {
            std::string path;
            std::string content;
            shaderc_include_result result{};
        };

        std::vector<std::string> search_paths;
    };
}

std::string utils::ShaderCompiler::get_shader_directory()
{
    const char* base_path = SDL_GetBasePath();
    if (base_path == nullptr)
    {
        fail(std::string("SDL_GetBasePath failed: ") + SDL_GetError());
    }

    return (std::filesystem::path(base_path) / "shaders").string();
}

std::vector<uint32_t> utils::ShaderCompiler::compile_glsl_to_spirv(const std::string& shader_path, VkShaderStageFlagBits stage)
{
    std::string source;
    if (!read_file(shader_path, source))
    {
        fail("Failed to open shader source: " + shader_path);
    }

    const std::filesystem::path path(shader_path);
    const std::filesystem::path shader_dir = path.parent_path();

    shaderc::CompileOptions options;
    options.SetIncluder(std::make_unique<FileIncluder>(std::vector<std::string>{
        shader_dir.string(),
        (shader_dir / "include").string()
    }));
    options.SetSourceLanguage(shaderc_source_language_glsl);
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);

#ifdef NDEBUG
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
    options.SetGenerateDebugInfo();
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
#endif

    const shaderc::Compiler compiler;
    const shaderc::SpvCompilationResult result =
        compiler.CompileGlslToSpv(source, to_shaderc_kind(stage), path.filename().string().c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        fail("Failed to compile " + shader_path + "\n" + result.GetErrorMessage());
    }

    if (result.GetNumWarnings() > 0)
    {
        std::cerr << "[ShaderCompiler] " << shader_path << ":\n" << result.GetErrorMessage();
    }

    std::cout << "Compiled shader: " << shader_path << std::endl;

    return {result.cbegin(), result.cend()};
}
