#include "Material.h"

#include <cassert>
#include <fstream>
#include <iostream>

#include "ShaderObject.h"
#include "../utils/FileUtils.h"
#include "../Config.h"

material::Material::Material(std::string material_name): material_name(std::move(material_name)), descriptor_set(nullptr), pipeline_layout(nullptr)
{
}

material::Material::~Material()
{
}

void material::Material::add_shader_object(std::unique_ptr<ShaderObject> shader_object)
{
    this->shader_object = std::move(shader_object);
}

void material::Material::add_pipeline_layout(VkPipelineLayout pipeline_layout)
{
    this->pipeline_layout = pipeline_layout;
}
