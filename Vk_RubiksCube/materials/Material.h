#pragma once
#include <memory>
#include <vulkan_core.h>
#include "../structs/MaterialValues.h"

namespace vkb
{
    struct DispatchTable;
}

class ShaderObject;

class Material
{

public:        
    Material();

    //load the shader code
    virtual bool init(const vkb::DispatchTable& disp, VkDescriptorSetLayout descriptor_set_layout, uint32_t descriptor_set_count, VkPushConstantRange push_constant_range, uint32_t push_constant_size);

private:

    size_t shaderCodeSizes[2]{};
    char* shaderCodes[2]{};

    std::unique_ptr<ShaderObject> shader_object;
    
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
};
