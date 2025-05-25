#pragma once

#include <string>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <vector>

#include "../structs/MaterialParams.h"
#include "../structs/Vertex.h"

struct TextureInfo;

namespace utils
{
    class ModelUtils
    {
    public:
        bool loadObj(const std::string& path,

                     std::vector<Vertex>& outVertices,
                     std::vector<uint32_t>& outIndices,

                     std::unordered_map<uint32_t, MaterialParams>& outMaterialParams, std::unordered_map<uint32_t, TextureInfo>&
                     outTextureInfo);

    private:
        bool getMaterialParams(uint32_t materialIndex, MaterialParams& outMaterialParams) const;

        bool getTextureInfo(uint32_t materialIndex, const std::string& textureDirectory, uint32_t& outTextureIndex, std::unordered_map<uint32_t, TextureInfo>& outTextureInfo);

        std::unordered_map<int, uint32_t> tinyObjMaterialIdToBufferIndex;
        std::unordered_map<std::string, uint32_t> texturePathToIndex;
        
        uint32_t nextMaterialBufferIndex = 0;
        uint32_t nextTextureIndex = 0;
        
        std::vector<tinyobj::material_t> materials;
    };
}
