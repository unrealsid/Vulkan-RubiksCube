#pragma once

#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H

#include <string>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <vector>

#include "../structs/MaterialParams.h"
#include "../structs/Vertex.h"
#include "../structs/GPU_Buffer.h"

struct EngineContext;

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
    class ModelLoaderUtils
    {
    public:
        //Loads an Obj
        //material_name_to_index global list of material name to slot ID
        bool load_obj(const std::string& path,

                     std::vector<Vertex>& outVertices,
                     std::vector<uint32_t>& outIndices,
                     
                     std::unordered_map<std::string, uint32_t>& material_name_to_index,
                     
                     std::unordered_map<uint32_t, MaterialParams>& material_params,
                     std::unordered_map<uint32_t, TextureInfo>& out_texture_info);

        bool set_texture_path_to_index(const std::unordered_map<std::string, uint32_t>& texture_path_to_index_);

        bool load_model_from_obj(const std::string& path,
                                 EngineContext& engine_context);

        [[nodiscard]] std::vector<Vertex> get_vertices() const
        {
            return vertices;
        }

        [[nodiscard]] std::vector<uint32_t> get_indices() const
        {
            return indices;
        }

        [[nodiscard]] GPU_Buffer get_vertex_buffer() const
        {
            return vertex_buffer;
        }

        [[nodiscard]] GPU_Buffer get_index_buffer() const
        {
            return index_buffer;
        }

        [[nodiscard]] std::unordered_map<uint32_t, std::pair<size_t, size_t>> get_material_index_ranges() const
        {
            return material_index_ranges;
        }
        
    private:

        //A map of material ID to index range for rendering 
        std::unordered_map<uint32_t, std::pair<size_t, size_t>> material_index_ranges;

        std::unordered_map<int, uint32_t> tiny_obj_material_id_to_buffer_index;
        std::unordered_map<std::string, uint32_t> texture_path_to_index;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        
        GPU_Buffer vertex_buffer = {};
        GPU_Buffer index_buffer = {}; 
        
        uint32_t next_material_buffer_index = 0;
        uint32_t next_texture_index = 0;
        
        std::vector<tinyobj::material_t> materials;
        
        bool get_material_params(uint32_t materialIndex, MaterialParams& outMaterialParams) const;

        bool get_texture_indices(uint32_t materialIndex, const std::string& textureDirectory, uint32_t& outTextureIndex, std::unordered_map<uint32_t, TextureInfo>& outTextureInfo);
    };
}

#endif