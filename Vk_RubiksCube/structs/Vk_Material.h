#pragma once
#include <memory>
#include <vulkan_core.h>

namespace material
{
    class ShaderObject;
}

struct Vk_Material
{
    std::unique_ptr<material::ShaderObject> shader_object;
    
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
};
