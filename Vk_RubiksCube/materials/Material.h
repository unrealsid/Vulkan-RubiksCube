#pragma once
#include <memory>
#include <vulkan_core.h>
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
        Material();

        //load the shader code
        void init();

        void add_shader_object(std::unique_ptr<ShaderObject> shader_object);
        void add_pipeline_layout(VkPipelineLayout pipeline_layout);

        VkPipelineLayout get_pipeline_layout() const { return pipeline_layout; }
        ShaderObject* get_shader_object() const { return shader_object.get(); }
        VkDescriptorSet& get_descriptor_set()  { return descriptor_set; }
    
    private: 
        std::unique_ptr<ShaderObject> shader_object;
        VkDescriptorSet descriptor_set;
        VkPipelineLayout pipeline_layout; 
    };
}