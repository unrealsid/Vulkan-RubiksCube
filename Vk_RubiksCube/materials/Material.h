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
    
    private: 
        std::unique_ptr<ShaderObject> shader_object;
        VkPipelineLayout pipeline_layout; 
    };
}