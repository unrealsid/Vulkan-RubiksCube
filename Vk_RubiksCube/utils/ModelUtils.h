#pragma once

#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H

#include <string>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <vector>

#include "../structs/MaterialParams.h"
#include "../structs/Vertex.h"
#include "../structs/Vk_Buffer.h"

namespace vulkan
{
    class DeviceManager;
}

namespace material
{
    class MaterialManager;
}

struct TextureInfo;

namespace utils
{
    class ModelUtils
    {
    public:
        //Loads an Obj
        //material_name_to_index global list of material name to slot ID
        bool loadObj(const std::string& path,

                     std::vector<Vertex>& outVertices,
                     std::vector<uint32_t>& outIndices,

                     std::unordered_map<std::string, uint32_t>& material_name_to_index,
                     
                     std::unordered_map<uint32_t, MaterialParams>& material_params,
                     std::unordered_map<uint32_t, TextureInfo>& outTextureInfo);

        bool set_texture_path_to_index(const std::unordered_map<std::string, uint32_t>& texture_path_to_index_);

        bool load_model_from_obj(const std::string& path,
                       const vulkan::DeviceManager& device_manager,
                       material::MaterialManager& material_manager);

        [[nodiscard]] std::vector<Vertex> getVertices() const
        {
            return vertices;
        }

        [[nodiscard]] std::vector<uint32_t> getIndices() const
        {
            return indices;
        }

    private:
        bool getMaterialParams(uint32_t materialIndex, MaterialParams& outMaterialParams) const;

        bool get_texture_indices(uint32_t materialIndex, const std::string& textureDirectory, uint32_t& outTextureIndex, std::unordered_map<uint32_t, TextureInfo>& outTextureInfo);

        std::unordered_map<int, uint32_t> tiny_obj_material_id_to_buffer_index;
        std::unordered_map<std::string, uint32_t> texture_path_to_index;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        Vk_Buffer vertex_buffer = {};
        Vk_Buffer index_buffer = {}; 
        
        uint32_t nextMaterialBufferIndex = 0;
        uint32_t nextTextureIndex = 0;
        
        std::vector<tinyobj::material_t> materials;
    };
}

#endif