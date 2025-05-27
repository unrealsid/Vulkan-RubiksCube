#pragma once

#ifndef VK_RENDER_DATA_H
#define VK_RENDER_DATA_H

#include <memory>
#include <unordered_map>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan_core.h>

#include "MaterialParams.h"
#include "Vk_MaterialData.h"
#include "Vk_SceneData.h"
#include "TextureInfo.h"
#include "Vk_DepthStencilImage.h"

struct Vertex;
class ShaderObject;

struct RenderData
{
    
    std::vector<uint32_t> primitiveMaterialIndices;
    std::unordered_map<uint32_t, MaterialParams> materialParams;
    std::unordered_map<uint32_t, TextureInfo> textureInfo;
};

#endif

