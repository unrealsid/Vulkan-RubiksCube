#include "ShaderObject.h"
#include "../Vk_RubiksCube.h"
#include "../Structs/Vertex.h"
#include <iostream>

ShaderObject::Shader::Shader(VkShaderStageFlagBits        stage_,
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
    shader_name = shader_name_;
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


void ShaderObject::Shader::destroy(VkDevice device)
{
    
}

VkPhysicalDeviceShaderObjectFeaturesEXT ShaderObject::create_shader_object_features()
{
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features{};
    shader_object_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT;
    shader_object_features.pNext = nullptr;
    shader_object_features.shaderObject = VK_TRUE;

    return shader_object_features;
}

void ShaderObject::build_linked_shaders(const vkb::DispatchTable& disp, ShaderObject::Shader* vert, ShaderObject::Shader* frag)
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

void ShaderObject::create_shaders(const Init& init, char* vertexShader, size_t vertShaderSize, char* fragmentShader, size_t fragShaderSize, const
                                  VkDescriptorSetLayout* pSetLayouts, uint32_t setLayoutCount, const VkPushConstantRange* pPushConstantRange, uint32_t
                                  pPushConstantCount)
{
	triangle_vert_shader = new Shader(VK_SHADER_STAGE_VERTEX_BIT,
                                        VK_SHADER_STAGE_FRAGMENT_BIT,
                                        "MeshShader", vertexShader,
                                        vertShaderSize, pSetLayouts, setLayoutCount, pPushConstantRange, pPushConstantCount);
                                        
    triangle_frag_shader = new Shader(VK_SHADER_STAGE_FRAGMENT_BIT,
                                    0,
                                    "MeshShader",
                                    fragmentShader,
                                    fragShaderSize, pSetLayouts, setLayoutCount, pPushConstantRange, pPushConstantCount);

    build_linked_shaders(init.disp, triangle_vert_shader, triangle_frag_shader);
}

void ShaderObject::bind_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer, const ShaderObject::Shader* shader)
{
    disp.cmdBindShadersEXT(cmd_buffer, 1, shader->get_stage(), shader->get_shader());
}

void ShaderObject::bind_material_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer) const
{
    bind_shader(disp, cmd_buffer, triangle_vert_shader);
    bind_shader(disp, cmd_buffer, triangle_frag_shader);
}

void ShaderObject::set_initial_state(const Init& init, VkCommandBuffer cmd_buffer)
{
    {
		// Set viewport and scissor to screen size
    	VkViewport viewport = {};
    	viewport.x = 0.0f;
    	viewport.y = 0.0f;
    	viewport.width = static_cast<float>(init.swapchain.extent.width);
    	viewport.height = static_cast<float>(init.swapchain.extent.height);
    	viewport.minDepth = 0.0f;
    	viewport.maxDepth = 1.0f;

    	VkRect2D scissor = {};
    	scissor.offset = { 0, 0 };
    	scissor.extent = init.swapchain.extent;
    	
		init.disp.cmdSetViewportWithCountEXT(cmd_buffer, 1, &viewport);
		init.disp.cmdSetScissorWithCountEXT(cmd_buffer, 1, &scissor);
    	init.disp.cmdSetCullModeEXT(cmd_buffer, VK_CULL_MODE_NONE);
    	init.disp.cmdSetFrontFaceEXT(cmd_buffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    	init.disp.cmdSetDepthTestEnableEXT(cmd_buffer, VK_TRUE);
    	init.disp.cmdSetDepthWriteEnableEXT(cmd_buffer, VK_TRUE);
    	init.disp.cmdSetDepthCompareOpEXT(cmd_buffer, VK_COMPARE_OP_LESS_OR_EQUAL);
    	init.disp.cmdSetPrimitiveTopologyEXT(cmd_buffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    	init.disp.cmdSetRasterizerDiscardEnableEXT(cmd_buffer, VK_FALSE);
    	init.disp.cmdSetPolygonModeEXT(cmd_buffer, VK_POLYGON_MODE_FILL);
    	init.disp.cmdSetRasterizationSamplesEXT(cmd_buffer, VK_SAMPLE_COUNT_1_BIT);
    	init.disp.cmdSetAlphaToCoverageEnableEXT(cmd_buffer, VK_FALSE);
    	init.disp.cmdSetDepthBiasEnableEXT(cmd_buffer, VK_FALSE);
		init.disp.cmdSetStencilTestEnableEXT(cmd_buffer, VK_FALSE);
    	init.disp.cmdSetPrimitiveRestartEnableEXT(cmd_buffer, VK_FALSE);

    	const VkSampleMask sample_mask = 0xFF;
    	init.disp.cmdSetSampleMaskEXT(cmd_buffer, VK_SAMPLE_COUNT_1_BIT, &sample_mask);

    	// Disable color blending
    	VkBool32 color_blend_enables= VK_FALSE;
    	init.disp.cmdSetColorBlendEnableEXT(cmd_buffer, 0, 1, &color_blend_enables);

    	// Use RGBA color write mask
    	VkColorComponentFlags color_component_flags = 0xF;
    	init.disp.cmdSetColorWriteMaskEXT(cmd_buffer, 0, 1, &color_component_flags);
	}

	//Vertex input
    {
    	// Get the vertex binding and attribute descriptions
    	auto bindingDescription = Vertex::getBindingDescription();
    	auto attributeDescriptions = Vertex::getAttributeDescriptions();

    	// Set the vertex input state using the descriptions
    	init.disp.cmdSetVertexInputEXT
        (
			cmd_buffer,
			1,                                                          // bindingCount = 1 (we have one vertex buffer binding)
			&bindingDescription,                                        // pVertexBindingDescriptions
			attributeDescriptions.size(),								// attributeCount
			attributeDescriptions.data()                                // pVertexAttributeDescriptions
		);
    }
	
	// This also requires setting blend equations
	// VkColorBlendEquationEXT colorBlendEquationEXT{};
	// init.disp.cmdSetColorBlendEquationEXT(cmd_buffer, 0, 1, &colorBlendEquationEXT);
	//
	//
	//
	// // Set depth state, the depth write. Don't enable depth bounds, bias, or stencil test.
	// init.disp.cmdSetDepthTestEnableEXT(cmd_buffer, VK_FALSE);
	// init.disp.cmdSetDepthCompareOpEXT(cmd_buffer, VK_COMPARE_OP_GREATER);
	// init.disp.cmdSetDepthBoundsTestEnableEXT(cmd_buffer, VK_FALSE);
	// init.disp.cmdSetDepthWriteEnableEXT(cmd_buffer, VK_FALSE);
	//
	// // Do not enable logic op
	// init.disp.cmdSetLogicOpEnableEXT(cmd_buffer, VK_FALSE);
	//
}
