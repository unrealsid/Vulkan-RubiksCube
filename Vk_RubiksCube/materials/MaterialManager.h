#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "ShaderObject.h"
#include "../structs/GPU_Buffer.h"
#include "../structs/Vk_Image.h"
#include "../structs/EngineContext.h"
#include "../structs/Vk_ShaderInfo.h"
#include "string"

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

        [[nodiscard]] std::unordered_map<std::string, std::unique_ptr<Material>>& get_materials()
        {
            return materials;
        }

        [[nodiscard]] VkDeviceAddress get_material_params_address() const
        {
            return material_params_address;
        }

        [[nodiscard]] std::string get_material_name_from_index(uint32_t index) const;
    
    private:
        //Store all kinds of materials (Translucent, Opaque etc)
        std::unordered_map<std::string, std::unique_ptr<Material>> materials;

        std::vector<Vk_Image> textures;

        //The buffer address for the material params
        VkDeviceAddress material_params_address;
        GPU_Buffer material_params_buffer;

        //Stores processed materials for each model
        std::unordered_map<std::string, uint32_t> material_name_to_index;

        //Stores material names to indices
        std::unordered_map<std::string, std::vector<uint32_t>> material_name_to_indices;
        
        //All unique materials to forward to the GPU
        std::unordered_map<uint32_t, MaterialParams> material_params;

        //Global texture Descriptor Info
        VkDescriptorSetLayout texture_descriptor_layout;
        VkDescriptorSet texture_descriptor_set;

        std::unordered_map<std::string, Vk_ShaderInfo> shader_info;
        EngineContext& engine_context; 

        void init_shaders();

        Material* get_or_create_material(const std::string& name);

        Material* create_material(const std::string& name);
    };
}