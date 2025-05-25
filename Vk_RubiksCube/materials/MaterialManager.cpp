#include "MaterialManager.h"

#include "Material.h"
#include "../utils/MemoryUtils.h"

MaterialManager::MaterialManager(vulkan::DeviceManager* device_manager_) : texture_descriptor_layout(nullptr),
                                                                           texture_descriptor_pool(nullptr)
{
    device_manager = device_manager_;
}

MaterialManager::~MaterialManager()
{
    
}

void MaterialManager::init()
{
    utils::MemoryUtils::createMaterialParamsBuffer(init, data);
    data.materialValues.materialParamsBufferAddress = utils::MemoryUtils::getBufferDeviceAddress(init.disp, data.materialValues.materialsBuffer.buffer);

    Vk_DescriptorUtils::setupDescriptors(init, data);
    
    auto material = std::make_unique<Material>();
    material->init()
}

void MaterialManager::addTexture(const Image& texture)
{
    textures.push_back(texture);
}
