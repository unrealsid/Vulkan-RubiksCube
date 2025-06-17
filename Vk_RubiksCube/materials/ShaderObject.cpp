#include "ShaderObject.h"
#include "../Vk_RubiksCube.h"
#include "../Structs/Vertex.h"
#include "../vulkan/SwapchainManager.h""
#include <iostream>
#include <utility>

material::ShaderObject::Shader::Shader(VkShaderStageFlagBits        stage_,
                             VkShaderStageFlags           next_stage_,
                             std::string                  shader_name_,
                             char*						  glsl_source,
                             size_t						  spirv_size,
                             const VkDescriptorSetLayout *pSetLayouts,
                             uint32_t                     setLayoutCount,
                             const VkPushConstantRange   *pPushConstantRange,
                             const  uint32_t			  pPushConstantCount)
{
    stage       = stage_;
    shader_name = std::move(shader_name_);
    next_stage  = next_stage_;
    spirv       = glsl_source;

    // Fill out the shader create info struct
    vk_shader_create_info.sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
    vk_shader_create_info.pNext                  = nullptr;
    vk_shader_create_info.flags                  = 0;
    vk_shader_create_info.stage                  = stage;
    vk_shader_create_info.nextStage              = next_stage;
    vk_shader_create_info.codeType               = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    vk_shader_create_info.codeSize               = spirv_size;
    vk_shader_create_info.pCode                  = spirv;
    vk_shader_create_info.pName                  = "main";
    vk_shader_create_info.setLayoutCount         = setLayoutCount;
    vk_shader_create_info.pSetLayouts            = pSetLayouts;
    vk_shader_create_info.pushConstantRangeCount = pPushConstantCount;
    vk_shader_create_info.pPushConstantRanges    = pPushConstantRange;
    vk_shader_create_info.pSpecializationInfo    = nullptr;
}


void material::ShaderObject::Shader::destroy(VkDevice device)
{
    
}

void material::ShaderObject::build_linked_shaders(const vkb::DispatchTable& disp, ShaderObject::Shader* vert, ShaderObject::Shader* frag)
{
    VkShaderCreateInfoEXT shader_create_infos[2];

    if (vert == nullptr || frag == nullptr)
    {
        std::cerr << ("build_linked_shaders failed with null vertex or fragment shader\n");
    }

    shader_create_infos[0] = vert->get_create_info();
    shader_create_infos[1] = frag->get_create_info();

    for (auto &shader_create : shader_create_infos)
    {
        shader_create.flags |= VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    }

    VkShaderEXT shaderEXTs[2];

    // Create the shader objects
    VkResult result = disp.createShadersEXT(
                                         2,
                                         shader_create_infos,
                                         nullptr,
                                         shaderEXTs);

    if (result != VK_SUCCESS)
    {
        std::cerr << ("vkCreateShadersEXT failed\n");
    }

    vert->set_shader(shaderEXTs[0]);
    frag->set_shader(shaderEXTs[1]);
}

void material::ShaderObject::create_shaders(const vkb::DispatchTable& disp, char* vertexShader, size_t vertShaderSize, char* fragmentShader, size_t fragShaderSize,
	const VkDescriptorSetLayout* pSetLayouts, uint32_t setLayoutCount,
	const VkPushConstantRange* pPushConstantRange, uint32_t pPushConstantCount)
{
	vert_shader = std::make_unique<Shader>(VK_SHADER_STAGE_VERTEX_BIT,
	                                      VK_SHADER_STAGE_FRAGMENT_BIT,
	                                      "MeshShader", vertexShader,
	                                      vertShaderSize, pSetLayouts, setLayoutCount, pPushConstantRange, pPushConstantCount);
                                        
    frag_shader = std::make_unique<Shader>(VK_SHADER_STAGE_FRAGMENT_BIT,
                                    0,
                                    "MeshShader",
                                    fragmentShader,
                                    fragShaderSize, pSetLayouts, setLayoutCount, pPushConstantRange, pPushConstantCount);

    build_linked_shaders(disp, vert_shader.get(), frag_shader.get());
}

void material::ShaderObject::destroy_shaders(VkDevice device)
{
}

void material::ShaderObject::bind_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer, const ShaderObject::Shader* shader)
{
    disp.cmdBindShadersEXT(cmd_buffer, 1, shader->get_stage(), shader->get_shader());
}

void material::ShaderObject::bind_material_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer) const
{
    bind_shader(disp, cmd_buffer, vert_shader.get());
    bind_shader(disp, cmd_buffer, frag_shader.get());
}
