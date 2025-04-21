#pragma once
#include <string>
#include <vector>
#include <vulkan_core.h>

#include "VkBootstrapDispatch.h"

struct Init;

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
        std::vector<uint32_t> spirv;

    public:
        Shader(){}
        Shader(VkShaderStageFlagBits        stage,
               VkShaderStageFlags           next_stage,
               std::string                  name,
               const std::vector<uint32_t>& glsl_source,
               const VkDescriptorSetLayout *pSetLayouts,
               const VkPushConstantRange   *pPushConstantRange);

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
    
    static VkPhysicalDeviceShaderObjectFeaturesEXT create_shader_object_features();

    ShaderObject() = default;
    ~ShaderObject() = default;

    void create_shaders(const vkb::DispatchTable& disp, const std::vector<uint32_t>& vertexShader, const std::vector<uint32_t>& fragmentShader);

    static void bind_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer, const ShaderObject::Shader *shader);
    void bind_material_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer) const;

    void set_initial_state(const Init& init, VkCommandBuffer cmd_buffer);

private:
    static void build_linked_shaders(const vkb::DispatchTable& disp, ShaderObject::Shader* vert, ShaderObject::Shader* frag);
    
    Shader* triangle_vert_shader;
    Shader* triangle_frag_shader;
};
