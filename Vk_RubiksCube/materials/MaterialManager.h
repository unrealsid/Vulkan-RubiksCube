#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "ShaderObject.h"
#include "../structs/GPU_Buffer.h"
#include "../structs/Vk_Image.h"
#include "../structs/EngineContext.h"

struct MaterialParams;

namespace vulkan
{
    class DeviceManager;
}

namespace material
{
    class Material;
    
    class MaterialManager
    {
    public:
        MaterialManager(EngineContext& engine_context);
        MaterialManager() = delete;
        ~MaterialManager();

        uint32_t max_materials;
        
        void init();

        //push a texture
        void add_texture(const Vk_Image& texture);

        void add_material(const std::string& name, std::unique_ptr<Material> material);

        [[nodiscard]] std::unordered_map<std::string, uint32_t>& get_material_name_to_index()
        {
            return material_name_to_index;
        }

        [[nodiscard]] std::unordered_map<uint32_t, MaterialParams>& get_material_params()
        {
            return material_params;
        }

        [[nodiscard]] std::unordered_map<std::string, std::unique_ptr<Material>> get_materials() const
        {
            return materials;
        }

        [[nodiscard]] VkDeviceAddress get_material_params_address() const
        {
            return material_params_address;
        }
    
    private:
        //Store all kinds of materials (Translucent, Opaque etc)
        std::unordered_map<std::string, std::unique_ptr<Material>> materials;

        std::vector<Vk_Image> textures;

        //The buffer address for the material params
        VkDeviceAddress material_params_address;
        GPU_Buffer material_params_buffer;

        //Global map of loaded material name to unique ID
        std::unordered_map<std::string, uint32_t> material_name_to_index;
        
        //All unique materials to forward to the GPU
        std::unordered_map<uint32_t, MaterialParams> material_params;

        //Global texture Descriptor Info
        VkDescriptorSetLayout texture_descriptor_layout;
        VkDescriptorSet texture_descriptor_set;

        void init_shaders();

        EngineContext& engine_context; 
    };
}