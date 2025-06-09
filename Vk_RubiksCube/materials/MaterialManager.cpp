#include "MaterialManager.h"

#include "Material.h"
#include "../structs/PushConstantBlock.h"
#include "../utils/FileUtils.h"
#include "../Config.h"
#include "../rendering/Renderer.h"
#include "../structs/Vk_ShaderInfo.h"
#include "../utils/DescriptorUtils.h"
#include "../utils/MemoryUtils.h"
#include "../vulkan/DeviceManager.h"
#include <cmath>

material::MaterialManager::MaterialManager(EngineContext& engine_context) : max_materials(1000),
                                                                            material_params_address(0), material_params_buffer({}),
                                                                            texture_descriptor_layout(nullptr),
                                                                            texture_descriptor_set(nullptr),
                                                                            engine_context(engine_context)
{
    shader_info =
    {
        {
            "Mesh Material", 
           {
               .vertex_shader_path = std::string(SHADER_PATH) + "/mesh_shader.vert.spv",
               .fragment_shader_path = std::string(SHADER_PATH) + "/mesh_shader.frag.spv"
           }
        }
    };
}

material::MaterialManager::~MaterialManager(){}

void material::MaterialManager::init()
{
    //Create a buffer to store material values
    //Assume we can create a maximum of 1000 materials
    VkDeviceSize material_buffer_size = sizeof(MaterialParams) * max_materials;

    auto device_manager = engine_context.device_manager.get();
    
    //Just create the buffer for now, but leave it empty
    utils::MemoryUtils::allocate_buffer_with_mapped_access(device_manager->get_allocator(), material_buffer_size, material_params_buffer);
    material_params_address = utils::MemoryUtils::get_buffer_device_address(engine_context.dispatch_table, material_params_buffer.buffer);
    
    //Setup Texture descriptors
    utils::DescriptorUtils::setup_texture_descriptors(engine_context.dispatch_table, textures, texture_descriptor_layout, texture_descriptor_set);

    init_shaders();
}

void material::MaterialManager::add_texture(const Vk_Image& texture)
{
    textures.push_back(texture);
}

void material::MaterialManager::add_material(const std::string& name, std::unique_ptr<Material> material)
{
    materials[name] = std::move(material);
}

std::string material::MaterialManager::get_material_name_from_index(uint32_t index) const
{
    for (const auto& [name, indices] : material_name_to_indices)
    {
        if (std::ranges::find(indices, index) != indices.end())
        {
            return name;
        }
    }
    return "";
}

void material::MaterialManager::init_shaders()
{
    auto device_manager = engine_context.device_manager.get();

    //Iterate over all materials
    for (const auto& [id, material_data] : material_params)
    {
        if (material_data.alpha.x == 1.0)
        {
            //Regular materials
            get_or_create_material("Mesh Material");
            material_name_to_indices["Mesh Material"].push_back(id);
        }
        if (material_data.alpha.x < 1.0)
        {
            //Translucent materials
            get_or_create_material("Translucent Material");
            material_name_to_indices["Translucent Material"].push_back(id);
        }
        else if (material_data.emissive.x == 1.0)
        {
            //Emissive materials
            get_or_create_material("Emissive Material");
            material_name_to_indices["Emissive Material"].push_back(id);
        }
    }
    
    //Populate the buffer with material data
    utils::MemoryUtils::createMaterialParamsBuffer(*device_manager, material_params, material_params_buffer);
}

material::Material* material::MaterialManager::get_or_create_material(const std::string& name)
{
    auto material_it = materials.find(name);
    if (material_it != materials.end())
    {
        return material_it->second.get();
    }
    return create_material(name);
}

material::Material* material::MaterialManager::create_material(const std::string& name)
{
    auto shader_info_data = shader_info.find(name);

    if (shader_info_data == shader_info.end())
    {
        std::cout << "Failed to find shader info for material: " << name << "\n";
        return nullptr;
    }
    
    //Setup push constants
    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(PushConstantBlock);
   
    size_t shaderCodeSizes[2]{};
    char* shaderCodes[2]{};

    utils::FileUtils::loadShader(shader_info_data->second.vertex_shader_path, shaderCodes[0], shaderCodeSizes[0]);
    utils::FileUtils::loadShader(shader_info_data->second.fragment_shader_path, shaderCodes[1], shaderCodeSizes[1]);

    auto shader_object = std::make_unique<ShaderObject>();
    shader_object->create_shaders(engine_context.dispatch_table,
        shaderCodes[0], shaderCodeSizes[0], shaderCodes[1], shaderCodeSizes[1],
        &texture_descriptor_layout, (texture_descriptor_layout != nullptr ? 1 : 0),
        &push_constant_range, 1);

    VkPipelineLayout pipeline_layout;
    
    //Create the pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = initializers::pipelineLayoutCreateInfo(&texture_descriptor_layout, (texture_descriptor_layout != nullptr ? 1 : 0), &push_constant_range, 1);
    engine_context.dispatch_table.createPipelineLayout(&pipelineLayoutInfo, VK_NULL_HANDLE, &pipeline_layout);
    
    //Create material 
    auto material = std::make_unique<Material>(name);
    material->add_shader_object(std::move(shader_object));
    material->add_pipeline_layout(pipeline_layout);
    material->add_descriptor_set(texture_descriptor_set);

    add_material(name, std::move(material));
    
    //TODO: VERY BAD. Remove Pliss
    return material.get();
}
