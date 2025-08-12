#pragma once
#include <vector>
#include <vk_mem_alloc.h>

#include "VkBootstrapDispatch.h"
#include "../structs/EngineContext.h"
#include "../structs/Vk_RenderData.h"
#include "../structs/Vk_Image.h"

struct GPU_SceneBuffer;
struct GPU_Buffer;
struct Vk_SceneData;

namespace utils
{
    class DescriptorUtils
    {
    public:
        static void setup_texture_descriptors(const vkb::DispatchTable& disp,
                                            const std::vector<Vk_Image>& textures,
                                            VkDescriptorSetLayout& outDescriptorSetLayout,
                                            VkDescriptorSet& outDescriptorSet);

        static void map_ubo(const EngineContext& engine_context, const Vk_SceneData& sceneDataUBO, GPU_SceneBuffer& gpu_scene_data);
    };
}

