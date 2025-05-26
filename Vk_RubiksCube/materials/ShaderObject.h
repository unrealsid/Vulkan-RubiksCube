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

        static void set_initial_state(vkb::DispatchTable& disp, const vulkan::SwapchainManager& swapchainManager, VkCommandBuffer cmd_buffer);

    private:
        static void build_linked_shaders(const vkb::DispatchTable& disp, ShaderObject::Shader* vert, ShaderObject::Shader* frag);
    
        std::unique_ptr<Shader> vert_shader;
        std::unique_ptr<Shader> frag_shader;
    };
}