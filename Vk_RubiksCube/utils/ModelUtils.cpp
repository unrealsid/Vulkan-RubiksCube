#include "ModelUtils.h"
#include <iostream>
#include "../structs/TextureInfo.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool VkUtils::ModelUtils::loadObj(const std::string& path,
                               std::vector<Vertex>& outVertices,
                               std::vector<uint32_t>& outIndices,
                               std::vector<uint32_t>& outPrimitiveMaterialIndices,
                               std::unordered_map<uint32_t, MaterialParams>& outMaterialParams,
                               std::unordered_map<uint32_t, TextureInfo>& outTextureInfo)
{
    // Extract the directory from the path for loading textures
    std::string textureDirectory = path.substr(0, path.find_last_of('/') + 1);

    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        return false;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto attrib = reader.GetAttrib();
    auto shapes = reader.GetShapes();
    materials = reader.GetMaterials();

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    outVertices.clear();
    outIndices.clear();
    outMaterialParams.clear();
    outTextureInfo.clear();
    outPrimitiveMaterialIndices.clear();
    tinyObjMaterialIdToBufferIndex.clear();
    texturePathToIndex.clear();
    nextMaterialBufferIndex = 0;
    nextTextureIndex = 0;

    // Handle the case of faces with no material assigned (-1 in tinyobjloader)
    int tiny_obj_no_material_id = -1;
    uint32_t default_buffer_index = nextMaterialBufferIndex++;
    tinyObjMaterialIdToBufferIndex[tiny_obj_no_material_id] = default_buffer_index;

    // Create a default MaterialParams for this index 0 (or the index assigned to -1)
    outMaterialParams[default_buffer_index] = MaterialParams(); // Default constructed MaterialParams
    
    // Create a default texture index for faces with no texture
    uint32_t defaultTextureIndex = UINT32_MAX;

    // Map all materials found in the MTL file(s)
    for (size_t i = 0; i < materials.size(); ++i)
    {
        int tinyObjId = static_cast<int>(i);
        uint32_t bufferIndex = nextMaterialBufferIndex++;
        tinyObjMaterialIdToBufferIndex[tinyObjId] = bufferIndex;

        // Get material params
        MaterialParams matParams;
        getMaterialParams(tinyObjId, matParams);

        // Store using contiguous index
        outMaterialParams[bufferIndex] = matParams;
        
        // Load texture info 
        uint32_t textureIndex = defaultTextureIndex;
        getTextureInfo(tinyObjId, textureDirectory, textureIndex, outTextureInfo);
    }

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            // Get the number of vertices for this original face (will be a multiple of 3 due to triangulation)
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Get the tinyobjloader material ID for this face
            int tiny_obj_material_id = shapes[s].mesh.material_ids[f];

            // Get our contiguous buffer index using the map
            uint32_t materialBufferIndex = tinyObjMaterialIdToBufferIndex.at(tiny_obj_material_id);
            
            // Get texture index for this material
            uint32_t textureIndex = defaultTextureIndex;
            if (tiny_obj_material_id >= 0) {
                // Try to find texture for this material
                const tinyobj::material_t& mat = materials[tiny_obj_material_id];
                
                // Check for diffuse texture (prioritizing diffuse texture)
                if (!mat.diffuse_texname.empty()) {
                    std::string texPath = textureDirectory + mat.diffuse_texname;
                    auto it = texturePathToIndex.find(texPath);
                    if (it != texturePathToIndex.end()) {
                        textureIndex = it->second;
                    }
                }
            }

            // We'll create a triangle for each face (triangulation)
            for (size_t v = 0; v < fv; v++)
            {
                Vertex vertex{};

                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
                tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
                tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];

                vertex.position = glm::vec3(vx, vy, vz);

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0)
                {
                    tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
                    tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
                    tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];

                    vertex.normal = glm::vec3(nx, ny, nz);
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0)
                {
                    tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
                    tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];

                    vertex.texCoords = glm::vec2(tx, ty);
                }

                // Set material and texture indices for the provoking vertex (first vertex of each triangle)
                if (v % 3 == 0)
                {
                    // First vertex of each triangle is the provoking vertex
                    vertex.materialIndex = materialBufferIndex;
                    vertex.textureIndex = textureIndex;
                }
                else
                {
                    // Use sentinel values for non-provoking vertices
                    vertex.materialIndex = UINT32_MAX;
                    vertex.textureIndex = UINT32_MAX;
                }

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(outVertices.size());
                    outVertices.push_back(vertex);
                }

                // Add index to outIndices
                outIndices.push_back(uniqueVertices[vertex]);
            }

            // We still maintain outPrimitiveMaterialIndices for compatibility
            size_t num_triangles_in_face = fv / 3;
            for (size_t i = 0; i < num_triangles_in_face; ++i)
            {
                outPrimitiveMaterialIndices.push_back(materialBufferIndex);
            }

            index_offset += fv;
        }
    }

    return true;
}

bool VkUtils::ModelUtils::getMaterialParams(uint32_t materialIndex, MaterialParams& outMaterialParams) const
{
    auto materialValues = materials[materialIndex];

    outMaterialParams.diffuse = glm::vec4(materialValues.diffuse[0], materialValues.diffuse[1], materialValues.diffuse[2], 0.0f);
    outMaterialParams.emissive = glm::vec4(materialValues.emission[0], materialValues.emission[1], materialValues.emission[2], 0.0f);
    outMaterialParams.shininess = glm::vec4(materialValues.shininess);

    outMaterialParams.alpha = glm::vec4(materialValues.dissolve);
    
    return true;
}

bool VkUtils::ModelUtils::getTextureInfo(uint32_t materialIndex, const std::string& textureDirectory,
    uint32_t& outTextureIndex, std::unordered_map<uint32_t, TextureInfo>& outTextureInfo)
{
    bool foundTexture = false;
    
    if (materialIndex >= materials.size())
    {
        return false;
    }
    
    const tinyobj::material_t& mat = materials[materialIndex];
    
    // Handle diffuse texture
    if (!mat.diffuse_texname.empty()) {
        std::string texPath = textureDirectory + mat.diffuse_texname;
        
        // Check if we've already processed this texture
        auto it = texturePathToIndex.find(texPath);
        if (it != texturePathToIndex.end())
        {
            // We already have this texture, just use its index
            outTextureIndex = it->second;
        }
        else
        {
            // New texture, assign a new index
            uint32_t texIndex = nextTextureIndex++;
            texturePathToIndex[texPath] = texIndex;
            outTextureInfo[texIndex] = TextureInfo(texPath, TextureInfo::Type::Diffuse);
            outTextureIndex = texIndex;
        }
        
        foundTexture = true;
    }
    
    // Similarly handle specular texture (just store the info, don't set as primary)
    if (!mat.specular_texname.empty()) {
        std::string texPath = textureDirectory + mat.specular_texname;
        
        // Only add if not already present
        auto it = texturePathToIndex.find(texPath);
        if (it == texturePathToIndex.end()) {
            uint32_t texIndex = nextTextureIndex++;
            texturePathToIndex[texPath] = texIndex;
            outTextureInfo[texIndex] = TextureInfo(texPath, TextureInfo::Type::Specular);
            
            // If we didn't find a diffuse texture, use this one as primary
            if (!foundTexture) {
                outTextureIndex = texIndex;
                foundTexture = true;
            }
        }
    }
    
    // Handle normal map
    if (!mat.normal_texname.empty()) {
        std::string texPath = textureDirectory + mat.normal_texname;
        
        // Only add if not already present
        auto it = texturePathToIndex.find(texPath);
        if (it == texturePathToIndex.end()) {
            uint32_t texIndex = nextTextureIndex++;
            texturePathToIndex[texPath] = texIndex;
            outTextureInfo[texIndex] = TextureInfo(texPath, TextureInfo::Type::Normal);
            
            // If we didn't find any texture yet, use this one as primary
            if (!foundTexture) {
                outTextureIndex = texIndex;
                foundTexture = true;
            }
        }
    }
    
    return foundTexture;
}
