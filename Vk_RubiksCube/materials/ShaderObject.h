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

    void create_shaders(const vkb::DispatchTable& disp, char* vertexShader, size_t vertShaderSize, char* fragmentShader, size_t fragShaderSize);

    static void bind_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer, const ShaderObject::Shader *shader);
    void bind_material_shader(const vkb::DispatchTable& disp, VkCommandBuffer cmd_buffer) const;

    void set_initial_state(const Init& init, VkCommandBuffer cmd_buffer);

private:
    static void build_linked_shaders(const vkb::DispatchTable& disp, ShaderObject::Shader* vert, ShaderObject::Shader* frag);
    
    Shader* triangle_vert_shader;
    Shader* triangle_frag_shader;
};
