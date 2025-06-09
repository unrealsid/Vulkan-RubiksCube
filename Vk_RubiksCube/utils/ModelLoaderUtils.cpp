#include "ModelLoaderUtils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include "../structs/TextureInfo.h"

#include "ImageUtils.h"
#include "MemoryUtils.h"


#include "../Config.h"
#include "../materials/MaterialManager.h"
#include "../structs/LoadedImageData.h"
#include "../vulkan/DeviceManager.h"

bool utils::ModelLoaderUtils::load_obj(const std::string& path,

                                std::unordered_map<std::string, uint32_t>& material_name_to_index,
                                
                                std::unordered_map<uint32_t, MaterialParams>& material_params,
                                std::unordered_map<uint32_t, TextureInfo>& out_texture_info)
{
    // Extract the directory from the path for loading textures
    std::string textureDirectory = path.substr(0, path.find_last_of('/') + 1);

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

    out_texture_info.clear();
    tiny_obj_material_id_to_buffer_index.clear();
    texture_path_to_index.clear();
    next_material_buffer_index = 0;
    next_texture_index = 0;
    
    //We need to start the next objects counter from the last objects index (Needed for muli-object renderings)
    next_material_buffer_index = material_params.size();

    // Create a default MaterialParams for this index 0 (or the index assigned to -1)
    //material_params[default_buffer_index] = MaterialParams(); // Default constructed MaterialParams
    
    // Create a default texture index for faces with no texture
    uint32_t defaultTextureIndex = UINT32_MAX;

    // Map all materials found in the MTL file(s)
    for (int tinyObjId = 0; tinyObjId < materials.size(); ++tinyObjId)
    {
        std::string materialName = materials[tinyObjId].name;
        uint32_t bufferIndex = 0;

        //Use an already stored buffer so we can reuse elements
        //Material was already loaded previously
        if (auto it = material_name_to_index.find(materialName); it != material_name_to_index.end())
        {
            bufferIndex = it->second;
        }
        else
        {
            bufferIndex = next_material_buffer_index++;

            // Get material params
            MaterialParams matParams;
            get_material_params(tinyObjId, matParams);

            // Store using contiguous index
            material_params[bufferIndex] = matParams;
        
            // Load texture info 
            uint32_t textureIndex = defaultTextureIndex;
            get_texture_indices(tinyObjId, textureDirectory, textureIndex, out_texture_info);

            material_name_to_index[materialName] = bufferIndex;
        }

        tiny_obj_material_id_to_buffer_index[tinyObjId] = bufferIndex;
    }
    
    // Loop over shapes
    for (auto& shape : shapes)
    {
        uint32_t current_start_index = 0;

        //Stores unique vertices
        std::unordered_map<Vertex, uint32_t> unique_vertices{};
        
        LoadedObject loaded_object;
        loaded_object.name = shape.name;
        
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
        {
            // Get the number of vertices for this original face (will be a multiple of 3 due to triangulation)
            size_t fv = shape.mesh.num_face_vertices[f];

            // Get the tinyobjloader material ID for this face
            int material_id = shape.mesh.material_ids[f];

            // Get our contiguous buffer index using the map
            uint32_t materialBufferIndex = tiny_obj_material_id_to_buffer_index.at(material_id);
            
            // Get texture index for this material
            uint32_t textureIndex = defaultTextureIndex;
            if (material_id >= 0)
            {
                // Try to find texture for this material
                const tinyobj::material_t& mat = materials[material_id];
                
                // Check for diffuse texture (prioritizing diffuse texture)
                if (!mat.diffuse_texname.empty())
                {
                    std::string texPath = textureDirectory + mat.diffuse_texname;
                    auto it = texture_path_to_index.find(texPath);
                    if (it != texture_path_to_index.end())
                    {
                        textureIndex = it->second;
                    }
                }
            }

            // We'll create a triangle for each face (triangulation)
            for (size_t v = 0; v < fv; v++)
            {
                Vertex vertex{};

                // access to vertex
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3*static_cast<size_t>(idx.vertex_index)+0];
                tinyobj::real_t vy = attrib.vertices[3*static_cast<size_t>(idx.vertex_index)+1];
                tinyobj::real_t vz = attrib.vertices[3*static_cast<size_t>(idx.vertex_index)+2];

                vertex.position = glm::vec3(vx, vy, vz);

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0)
                {
                    tinyobj::real_t nx = attrib.normals[3*static_cast<size_t>(idx.normal_index)+0];
                    tinyobj::real_t ny = attrib.normals[3*static_cast<size_t>(idx.normal_index)+1];
                    tinyobj::real_t nz = attrib.normals[3*static_cast<size_t>(idx.normal_index)+2];

                    vertex.normal = glm::vec3(nx, ny, nz);
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0)
                {
                    tinyobj::real_t tx = attrib.texcoords[2*static_cast<size_t>(idx.texcoord_index)+0];
                    tinyobj::real_t ty = attrib.texcoords[2*static_cast<size_t>(idx.texcoord_index)+1];

                    vertex.texCoords = glm::vec2(tx, ty);
                }

                // Set material and texture indices for the provoking vertex (first vertex of each triangle)
                if (v % 3 == 0)
                {
                    // The first vertex of each triangle is the provoking vertex
                    vertex.materialIndex = materialBufferIndex;
                    vertex.textureIndex = textureIndex;
                }
                else
                {
                    // Use sentinel values for non-provoking vertices
                    vertex.materialIndex = UINT32_MAX;
                    vertex.textureIndex = UINT32_MAX;
                }

                if (unique_vertices.count(vertex) == 0)
                {
                    unique_vertices[vertex] = static_cast<uint32_t>(loaded_object.vertices.size());
                    loaded_object.vertices.push_back(vertex);
                }

                // Add index to outIndices
                loaded_object.indices.push_back(unique_vertices[vertex]);
            }

            // Update material index ranges
            uint32_t current_end_index = loaded_object.indices.size();
            if (loaded_object.material_index_ranges.find(material_id) == loaded_object.material_index_ranges.end())
            {
                loaded_object.material_index_ranges[material_id] = { current_start_index, current_end_index };
            }
            else
            {
                loaded_object.material_index_ranges[material_id].second = current_end_index;
            }

            assert(current_end_index > current_start_index);
            
            current_start_index = current_end_index;

            index_offset += fv;
        }

        loaded_objects.push_back(loaded_object);
    }

    return true;
}

bool utils::ModelLoaderUtils::set_texture_path_to_index(const std::unordered_map<std::string, uint32_t>& texture_path_to_index_)
{
    texture_path_to_index = texture_path_to_index_;
    return true;
}

bool utils::ModelLoaderUtils::load_model_from_obj(const std::string& path,
                                                  EngineContext& engine_context)
{
    auto material_manager = engine_context.material_manager.get();
    
    //Texture data
    std::unordered_map<uint32_t, TextureInfo> out_texture_info;

    //Load the object from the obj file
    load_obj(std::string(RESOURCE_PATH) + path, material_manager->get_material_name_to_index(), material_manager->get_material_params(), out_texture_info);

    //Process textures
    for (const auto& texture : out_texture_info)
    {
        LoadedImageData image_data = ImageUtils::load_image_data(texture.second.path);
        material_manager->add_texture(ImageUtils::create_texture_image(engine_context, image_data));
    }
    
    //Create Model index and vertex buffers
    for (auto& loaded_object : loaded_objects)
    {
        MemoryUtils::create_vertex_and_index_buffers(engine_context,
            loaded_object.vertices,loaded_object.indices,
            loaded_object.vertex_buffer, loaded_object.index_buffer);

        std::cout << "Loaded object: " << loaded_object.name << std::endl;
        std::cout << "Vertices: " << loaded_object.vertices.size() << std::endl;
        std::cout << "Indices: " << loaded_object.indices.size() << std::endl;
    }

    return true;   
}

bool utils::ModelLoaderUtils::get_material_params(uint32_t materialIndex, MaterialParams& outMaterialParams) const
{
    auto materialValues = materials[materialIndex];

    outMaterialParams.diffuse = glm::vec4(materialValues.diffuse[0], materialValues.diffuse[1], materialValues.diffuse[2], 0.0f);
    outMaterialParams.emissive = glm::vec4(materialValues.emission[0], materialValues.emission[1], materialValues.emission[2], 0.0f);
    outMaterialParams.shininess = glm::vec4(materialValues.shininess);

    outMaterialParams.alpha = glm::vec4(materialValues.dissolve);
    
    return true;
}

bool utils::ModelLoaderUtils::get_texture_indices(uint32_t materialIndex, const std::string& textureDirectory,
    uint32_t& outTextureIndex, std::unordered_map<uint32_t, TextureInfo>& outTextureInfo)
{
    bool foundTexture = false;
    
    if (materialIndex >= materials.size())
    {
        return false;
    }
    
    const tinyobj::material_t& mat = materials[materialIndex];
    
    // Handle diffuse texture
    if (!mat.diffuse_texname.empty())
    {
        std::string texPath = textureDirectory + mat.diffuse_texname;
        
        // Check if we've already processed this texture
        auto it = texture_path_to_index.find(texPath);
        if (it != texture_path_to_index.end())
        {
            // We already have this texture, just use its index
            outTextureIndex = it->second;
        }
        else
        {
            // New texture, assign a new index
            uint32_t texIndex = next_texture_index++;
            texture_path_to_index[texPath] = texIndex;
            outTextureInfo[texIndex] = TextureInfo(texPath, TextureInfo::Type::Diffuse);
            outTextureIndex = texIndex;
        }
        
        foundTexture = true;
    }
    
    // Similarly handle specular texture (just store the info, don't set as primary)
    if (!mat.specular_texname.empty()) {
        std::string texPath = textureDirectory + mat.specular_texname;
        
        // Only add if not already present
        auto it = texture_path_to_index.find(texPath);
        if (it == texture_path_to_index.end())
        {
            uint32_t texIndex = next_texture_index++;
            texture_path_to_index[texPath] = texIndex;
            outTextureInfo[texIndex] = TextureInfo(texPath, TextureInfo::Type::Specular);
            
            // If we didn't find a diffuse texture, use this one as primary
            if (!foundTexture)
            {
                outTextureIndex = texIndex;
                foundTexture = true;
            }
        }
    }
    
    // Handle normal map
    if (!mat.normal_texname.empty())
    {
        std::string texPath = textureDirectory + mat.normal_texname;
        
        // Only add if not already present
        auto it = texture_path_to_index.find(texPath);
        if (it == texture_path_to_index.end())
        {
            uint32_t texIndex = next_texture_index++;
            texture_path_to_index[texPath] = texIndex;
            outTextureInfo[texIndex] = TextureInfo(texPath, TextureInfo::Type::Normal);
            
            // If we didn't find any texture yet, use this one as primary
            if (!foundTexture)
            {
                outTextureIndex = texIndex;
                foundTexture = true;
            }
        }
    }
    
    return foundTexture;
}
