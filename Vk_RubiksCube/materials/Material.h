#pragma once
#include <memory>
#include <string>
#include <vulkan/vulkan_core.h>
#include "../structs/Vk_MaterialData.h"

namespace vkb
{
    struct DispatchTable;
}

namespace material
{
    class ShaderObject;
    
    class Material
    {

    public:        
        Material(std::string material_name);

        ~Material();

        //load the shader code
        void init();

        void add_shader_object(std::unique_ptr<ShaderObject> shader_object);
        void add_pipeline_layout(VkPipelineLayout pipeline_layout);
        void add_descriptor_set(VkDescriptorSet descriptor_set);

        VkPipelineLayout get_pipeline_layout() const { return pipeline_layout; }
        ShaderObject* get_shader_object() const { return shader_object.get(); }
        VkDescriptorSet& get_descriptor_set()  { return descriptor_set; }
        std::string get_material_name() const { return material_name; }
    
    private: 
        std::unique_ptr<ShaderObject> shader_object;
        VkDescriptorSet descriptor_set;
        VkPipelineLayout pipeline_layout;

        std::string material_name;
    };
}