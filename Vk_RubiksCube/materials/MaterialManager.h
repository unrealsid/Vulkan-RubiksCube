#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "ShaderObject.h"

namespace vulkan
{
    class DeviceManager;
}

struct Image;
class Material;

class MaterialManager
{
public:
    MaterialManager(vulkan::DeviceManager* device_manager_);
    MaterialManager() = default;
    ~MaterialManager();

    void init();

    //push a texture
    void addTexture(const Image& texture);
    
private:    
    std::unordered_map<std::string, std::unique_ptr<Material>> materials;
    std::vector<Image> textures;
    vulkan::DeviceManager* device_manager;

    VkDescriptorSetLayout texture_descriptor_layout;
    VkDescriptorPool texture_descriptor_pool;
};
