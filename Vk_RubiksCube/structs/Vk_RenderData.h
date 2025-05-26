#pragma once

#ifndef VK_RENDER_DATA_H
#define VK_RENDER_DATA_H

#include <memory>
#include <unordered_map>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan_core.h>

#include "MaterialParams.h"
#include "GPUMaterialData.h"
#include "SceneData.h"
#include "TextureInfo.h"
#include "Vk_DepthStencil_Image.h"

struct Vertex;
class ShaderObject;

struct RenderData
{
    
    
    VkBuffer vertexBuffer;
    VmaAllocation vertexBufferAllocation;
    VkBuffer indexBuffer;
    VmaAllocation indexBufferAllocation;

    std::vector<uint32_t> outIndices;
    std::vector<Vertex> outVertices;

    SceneData sceneData;
    GPUMaterialData materialValues;

    std::vector<uint32_t> primitiveMaterialIndices;
    std::unordered_map<uint32_t, MaterialParams> materialParams;
    std::unordered_map<uint32_t, TextureInfo> textureInfo;

    DepthStencil_Image depthStencilImage;
};

#endif

