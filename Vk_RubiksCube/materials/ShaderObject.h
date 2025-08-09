#pragma once
#include <memory>
#include <string>
#include <vector>
#include <vulkan_core.h>

#include "VkBootstrapDispatch.h"

namespace vulkan
{
    class SwapchainManager;
}

struct Init;

namespace material
{
	class ShaderObject
	{
	public:
		class Shader
		{
			VkShaderStageFlagBits stage{};
			VkShaderStageFlags    next_stage{};
			VkShaderEXT           shader      = VK_NULL_HANDLE;
			std::string           shader_name = "shader";
			VkShaderCreateInfoEXT vk_shader_create_info{};
			char* spirv = nullptr;
			size_t spirv_size = 0;

		public:
			Shader(){}
			Shader(VkShaderStageFlagBits        stage,
				   VkShaderStageFlags           next_stage,
				   std::string                  name,
				   char*                        glsl_source,
				   size_t                       spir_size,
				   const VkDescriptorSetLayout *pSetLayouts,
				   uint32_t                     setLayoutCount,
				   const VkPushConstantRange   *pPushConstantRange,
				   const  uint32_t			    pPushConstantCount);

			std::string get_name()
			{
				return shader_name;
			}

			VkShaderCreateInfoEXT get_create_info() const
			{
				return vk_shader_create_info;
			}

			const VkShaderEXT *get_shader() const
			{
				return &shader;
			}

			const VkShaderStageFlagBits *get_stage() const
			{
				return &stage;
			}

			const VkShaderStageFlags *get_next_stage() const
			{
				return &next_stage;
			}

			void set_shader(VkShaderEXT _shader)
			{
				shader = _shader;
			}

			void destroy(VkDevice device);
		};

		ShaderObject() = default;
		~ShaderObject() = default;

		void create_shaders(const vkb::DispatchTable& disp,
			char* vertexShader, size_t vertShaderSize,
			char* fragmentShader, size_t fragShaderSize,
			const VkDescriptorSetLayout *pSetLayouts, uint32_t setLayoutCount,
			const VkPushConstantRange *pPushConstantRange, uint32_t pPushConstantCount);
    
		void destroy_shaders(VkDevice device);

		static void bind_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer, const ShaderObject::Shader *shader);
		void bind_material_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer) const;

		template<size_t N>
		static void set_initial_state(vkb::DispatchTable& disp, VkExtent2D viewport_extent, VkCommandBuffer cmd_buffer, VkVertexInputBindingDescription2EXT
		                              vertex_input_binding, std::array<VkVertexInputAttributeDescription2EXT, N> input_attribute_description,
		                              VkExtent2D scissor_extents,
		                              VkOffset2D scissor_offset);

	private:
		static void build_linked_shaders(const vkb::DispatchTable& disp, ShaderObject::Shader* vert, ShaderObject::Shader* frag);
    
		std::unique_ptr<Shader> vert_shader;
		std::unique_ptr<Shader> frag_shader;
	};

	template <size_t N>
	void ShaderObject::set_initial_state(vkb::DispatchTable& disp, VkExtent2D viewport_extent, VkCommandBuffer cmd_buffer,
	                                     VkVertexInputBindingDescription2EXT vertex_input_binding,
	                                     std::array<VkVertexInputAttributeDescription2EXT, N> input_attribute_description, VkExtent2D scissor_extents, VkOffset2D scissor_offset = {.x = 0, .y = 0 })
	{
		{
			// Set viewport and scissor
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(viewport_extent.width);
			viewport.height = static_cast<float>(viewport_extent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = scissor_offset;
			scissor.extent = scissor_extents;
    	
			disp.cmdSetViewportWithCountEXT(cmd_buffer, 1, &viewport);
			disp.cmdSetScissorWithCountEXT(cmd_buffer, 1, &scissor);

			disp.cmdSetScissor(cmd_buffer, 0, 1, &scissor);
			
			disp.cmdSetCullModeEXT(cmd_buffer, VK_CULL_MODE_NONE);
			disp.cmdSetFrontFaceEXT(cmd_buffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
			disp.cmdSetDepthTestEnableEXT(cmd_buffer, VK_TRUE);
			disp.cmdSetDepthWriteEnableEXT(cmd_buffer, VK_TRUE);
			disp.cmdSetDepthCompareOpEXT(cmd_buffer, VK_COMPARE_OP_LESS);
			disp.cmdSetPrimitiveTopologyEXT(cmd_buffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
			disp.cmdSetRasterizerDiscardEnableEXT(cmd_buffer, VK_FALSE);
			disp.cmdSetPolygonModeEXT(cmd_buffer, VK_POLYGON_MODE_FILL);
			disp.cmdSetRasterizationSamplesEXT(cmd_buffer, VK_SAMPLE_COUNT_1_BIT);
			disp.cmdSetAlphaToCoverageEnableEXT(cmd_buffer, VK_FALSE);
			disp.cmdSetDepthBiasEnableEXT(cmd_buffer, VK_FALSE);
			disp.cmdSetStencilTestEnableEXT(cmd_buffer, VK_FALSE);
			disp.cmdSetPrimitiveRestartEnableEXT(cmd_buffer, VK_FALSE);

			const VkSampleMask sample_mask = 0xFF;
			disp.cmdSetSampleMaskEXT(cmd_buffer, VK_SAMPLE_COUNT_1_BIT, &sample_mask);

			// Disable color blending
			VkBool32 color_blend_enables= VK_FALSE;
			disp.cmdSetColorBlendEnableEXT(cmd_buffer, 0, 1, &color_blend_enables);

			// Use RGBA color write mask
			VkColorComponentFlags color_component_flags = 0xF;
			disp.cmdSetColorWriteMaskEXT(cmd_buffer, 0, 1, &color_component_flags);
		}

		//Vertex input
		{
			// Get the vertex binding and attribute descriptions
			auto bindingDescription = vertex_input_binding;  //Vertex::get_binding_description();
			auto attributeDescriptions = input_attribute_description; //Vertex::get_attribute_descriptions();

			// Set the vertex input state using the descriptions
			disp.cmdSetVertexInputEXT
			(
				cmd_buffer,
				1,                                                          // bindingCount = 1 (we have one vertex buffer binding)
				&bindingDescription,                                        // pVertexBindingDescriptions
				attributeDescriptions.size(),								// attributeCount
				attributeDescriptions.data()                                // pVertexAttributeDescriptions
			);
		}
	};
}
