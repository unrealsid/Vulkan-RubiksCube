#include "ModelUtils.h"
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

bool VkUtils::ModelUtils::loadObj(const std::string& path,
                                  std::vector<Vertex>& outVertices,
                                  std::vector<uint32_t>& outIndices,
                                  
                                  std::vector<uint32_t>& outPrimitiveMaterialIndices,
                                  std::unordered_map<uint32_t, MaterialParams>& outMaterialParams)
{
    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path, reader_config))
    {
        if (!reader.Error().empty())
        {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
    
        return false;
    }

    if (!reader.Warning().empty())
    {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto attrib = reader.GetAttrib();
    auto shapes = reader.GetShapes();
    materials = reader.GetMaterials();
    
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    outVertices.clear();
    outIndices.clear();
    outMaterialParams.clear();
    outPrimitiveMaterialIndices.clear();
    tinyObjMaterialIdToBufferIndex.clear(); 
    nextMaterialBufferIndex = 0;

    // Handle the case of faces with no material assigned (-1 in tinyobjloader)
    int tiny_obj_no_material_id = -1;
    uint32_t default_buffer_index = nextMaterialBufferIndex++;
    tinyObjMaterialIdToBufferIndex[tiny_obj_no_material_id] = default_buffer_index;
    
    // Create a default MaterialParams for this index 0 (or the index assigned to -1)
    outMaterialParams[default_buffer_index] = MaterialParams(); // Default constructed MaterialParams

    // Map all materials found in the MTL file(s)
    for (size_t i = 0; i < materials.size(); ++i)
    {
        int tinyObjId = static_cast<int>(i);
        uint32_t bufferIndex = nextMaterialBufferIndex++;
        tinyObjMaterialIdToBufferIndex[tinyObjId] = bufferIndex;

        //Get material params 
        MaterialParams matParams;
        getMaterialParams(tinyObjId, matParams);

        // Store using contiguous index
        outMaterialParams[bufferIndex] = matParams; 
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
            // Use .at() for safe lookup, or handle potential missing keys if necessary
            uint32_t materialBufferIndex = tinyObjMaterialIdToBufferIndex.at(tiny_obj_material_id);

            // Loop over vertices in the face.
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

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(outVertices.size());
                    outVertices.push_back(vertex);
                }

                // --- Add index to outIndices ---
                outIndices.push_back(uniqueVertices[vertex]);

                // Optional: vertex colors
                // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
            }

            size_t num_triangles_in_face = fv / 3;
            for (size_t i = 0; i < num_triangles_in_face; ++i)
            {
                outPrimitiveMaterialIndices.push_back(materialBufferIndex);
            }
            
            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
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

/*bool ModelUtils::getTextureParams(uint32_t materialIndex, const std::string& textureDirectory, TextureParams& outTextureParams, std::unordered_map<std::string, GLuint>& outLoadedTextures) const
{
    if (materialIndex < materials.size() &&
        !textureDirectory.empty())
    {
        const tinyobj::material_t& mat = materials[materialIndex];

        // Load textures (diffuse, specular, normal, etc.)
        if (!mat.diffuse_texname.empty() &&
            outLoadedTextures.find(mat.diffuse_texname) == outLoadedTextures.end())
        {
            outTextureParams.bIsValid = true;

            auto texturePath = textureDirectory + mat.diffuse_texname;
            
            GLuint diffuseTexture = TextureManager::getTexture(texturePath);
            outLoadedTextures[texturePath] = diffuseTexture;
                        
            outTextureParams.diffuseTexturePath = texturePath;
            outTextureParams.diffuseTextureID = new GLuint(diffuseTexture);
        }

        if (!mat.specular_texname.empty() &&
            outLoadedTextures.find(mat.specular_texname) == outLoadedTextures.end())
        {
            outTextureParams.bIsValid = true;

            auto texturePath = textureDirectory + mat.specular_texname;
            
            GLuint specularTexture = TextureManager::getTexture(texturePath);
            outLoadedTextures[texturePath] = specularTexture;

            outTextureParams.specularTexturePath = texturePath;
            outTextureParams.specularTextureID = new GLuint(specularTexture);
        }

        if (!mat.normal_texname.empty() &&
            outLoadedTextures.find(mat.normal_texname) == outLoadedTextures.end())
        {
            outTextureParams.bIsValid = true;

            auto texturePath = textureDirectory + mat.normal_texname;
            
            GLuint normalTexture = TextureManager::getTexture(texturePath);
            outLoadedTextures[texturePath] = normalTexture;

            outTextureParams.normalTexturePath = texturePath;
            outTextureParams.normalTextureID = new GLuint(normalTexture);
        }
    }

    return outTextureParams.bIsValid;
}
*/
