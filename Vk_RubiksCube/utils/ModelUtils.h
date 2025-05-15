#pragma once

#include <string>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <vector>

#include "../structs/MaterialParams.h"
#include "../structs/Vertex.h"

namespace VkUtils
{
    class ModelUtils
    {
    public:
        bool loadObj(const std::string &path,
                 
                     std::vector<Vertex>& outVertices,
                     std::vector<uint32_t>& outIndices,

                     std::vector<uint32_t>& outPrimitiveMaterialIndices,
                     std::unordered_map<uint32_t, MaterialParams>& outMaterialParams);

    private:
        bool getMaterialParams(uint32_t materialIndex, MaterialParams& outMaterialParams) const;

        //bool getTextureParams(uint32_t materialIndex, const std::string& textureDirectory, TextureParams& outTextureParams, std::unordered_map<std::string, GLuint>& outLoadedTextures) const;

        std::unordered_map<int, uint32_t> tinyObjMaterialIdToBufferIndex;
        uint32_t nextMaterialBufferIndex = 0; 
        
        std::vector<tinyobj::material_t> materials;
    };
}