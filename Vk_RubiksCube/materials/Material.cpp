#include "Material.h"

#include <cassert>
#include <fstream>
#include <iostream>

#include "ShaderObject.h"
#include "../utils/FileUtils.h"
#include "../Config.h"

Material::Material()
{
    pipeline_layout = nullptr;
    graphics_pipeline = nullptr;
    shader_object = nullptr;
    //materialValues = MaterialValues();
}

bool Material::init(const vkb::DispatchTable& disp, VkDescriptorSetLayout descriptor_set_layout, uint32_t descriptor_set_count, VkPushConstantRange
                    push_constant_range, uint32_t push_constant_size)
{
    FileUtils::loadShader(std::string(SHADER_PATH) + "/mesh_shader.vert.spv", shaderCodes[0], shaderCodeSizes[0]);
    FileUtils::loadShader(std::string(SHADER_PATH) + "/mesh_shader.frag.spv", shaderCodes[1], shaderCodeSizes[1]);

    shader_object = std::make_unique<ShaderObject>();
    shader_object->create_shaders(disp, shaderCodes[0], shaderCodeSizes[0], shaderCodes[1], shaderCodeSizes[1],
        &descriptor_set_layout, descriptor_set_count,
        &push_constant_range, push_constant_size);
    
    return true;
}
