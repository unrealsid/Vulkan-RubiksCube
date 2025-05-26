#include "Material.h"

#include <cassert>
#include <fstream>
#include <iostream>

#include "ShaderObject.h"
#include "../utils/FileUtils.h"
#include "../Config.h"

material::Material::Material()
{
    //materialValues = GPUMaterialData();K
}

void  material::Material::init()
{
}

void material::Material::add_shader_object(std::unique_ptr<ShaderObject> shader_object_)
{
    shader_object = std::move(shader_object_);
}
