#include "DeviceManager.h"

#include <iostream>
#include <vulkan_core.h>
#include <GLFW/glfw3.h>

#include "VkBootstrap.h"
#include "Features/VulkanFeatureActivator.h"
#include "../rendering/Vk_DynamicRendering.h"
#include "../platform/WindowManager.h"
#include "../structs/EngineContext.h"

vulkan::DeviceManager::DeviceManager()
{
}

vulkan::DeviceManager::~DeviceManager()
{
    
}

bool vulkan::DeviceManager::device_init(EngineContext& engine_context)
{
    // Create the disable feature struct
    VkValidationFeatureDisableEXT disables[] =
    {
        VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT
    };
    
    vkb::InstanceBuilder instance_builder;
    auto instance_ret = instance_builder.
        set_minimum_instance_version(VK_API_VERSION_1_4)
        .use_default_debug_messenger()
        .add_validation_feature_disable(*disables)
        //.enable_layer("VK_LAYER_KHRONOS_shader_object")
        .enable_extension(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME)
        .enable_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)
        .enable_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
        .request_validation_layers()
        .build();
    
    if (!instance_ret)
    {
        std::cout << instance_ret.error().message() << "\n";
        return false;
    }
    instance = instance_ret.value();

    engine_context.instance_dispatch_table = instance.make_table();

    surface = create_surface_GLFW(engine_context);

    VkPhysicalDeviceFeatures features = {};
    features.geometryShader = VK_FALSE;
    features.tessellationShader = VK_FALSE;

    vkb::PhysicalDeviceSelector phys_device_selector(instance);
    auto phys_device_ret = phys_device_selector
        .add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
        .add_required_extension(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME)
        .add_required_extension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)
        .add_required_extension(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME)
        .add_required_extension(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)
        .add_required_extension(VK_EXT_SHADER_OBJECT_EXTENSION_NAME)
        .add_required_extension(VK_KHR_MULTIVIEW_EXTENSION_NAME)
        .add_required_extension(VK_KHR_MAINTENANCE_2_EXTENSION_NAME)
        .add_required_extension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)
        .add_required_extension(VK_KHR_DEVICE_GROUP_EXTENSION_NAME)
        .add_required_extension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) 
        .add_required_extension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)
        .add_required_extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME)
        .add_required_extension(VK_KHR_MAINTENANCE3_EXTENSION_NAME)
        .add_required_extension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)
        .add_required_extension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)
        //.add_required_extension(VK_KHR_MAINTENANCE_6_EXTENSION_NAME)
        .set_required_features(features)
        .set_surface(surface)
        .select();

    auto dynamic_rendering_features = VulkanFeatureActivator::createDynamicRenderingFeatures();
    auto shader_object_features = VulkanFeatureActivator::createShaderObjectFeatures();
    auto device_memory_features = VulkanFeatureActivator::createPhysicaDeviceBufferAddress();
    auto descriptorIndexingFeatures = VulkanFeatureActivator::createPhysicalDeviceDescriptorIndexingFeatures();
    
    if (!phys_device_ret)
    {
        std::cout << phys_device_ret.error().message() << "\n";
        return false;
    }
    const vkb::PhysicalDevice& p_device = phys_device_ret.value();

    vkb::DeviceBuilder device_builder{ p_device };
    auto device_ret = device_builder
        .add_pNext(&dynamic_rendering_features)
        .add_pNext(&shader_object_features)
        .add_pNext(&device_memory_features)
        .add_pNext(&descriptorIndexingFeatures)
        .build();
    
    if (!device_ret)
    {
        std::cout << device_ret.error().message() << "\n";
        return false;
    }

    device = device_ret.value();
    physical_device = p_device;
    engine_context.dispatch_table = device.make_table();

    return true;
}

VkSurfaceKHR vulkan::DeviceManager::create_surface_GLFW(const EngineContext& engine_context, const VkAllocationCallbacks* allocator)
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult err = glfwCreateWindowSurface(instance, engine_context.window_manager->getWindow(), allocator, &surface);
    if (err)
    {
        const char* error_msg;
        int ret = glfwGetError(&error_msg);
        if (ret != 0)
        {
            std::cout << ret << " ";
            if (error_msg != nullptr) std::cout << error_msg;
            std::cout << "\n";
        }
        surface = VK_NULL_HANDLE;
    }
    return surface;
}

bool vulkan::DeviceManager::get_queues()
{
    auto gq = device.get_queue(vkb::QueueType::graphics);
    if (!gq.has_value())
    {
        std::cout << "failed to get graphics queue: " << gq.error().message() << "\n";
        return false;
    }
    graphics_queue = gq.value();

    auto pq = device.get_queue(vkb::QueueType::present);
    if (!pq.has_value())
    {
        std::cout << "failed to get present queue: " << pq.error().message() << "\n";
        return false;
    }
    present_queue = pq.value();
    return true;
}

// bool vulkan::DeviceManager::create_graphics_pipeline()
// {
//      //Buffer device address
//     Vk_DescriptorUtils::createBuffer(init, sizeof(SceneData), data.sceneData.sceneBuffer);
//     data.sceneData.sceneBufferAddress = vmaUtils::getBufferDeviceAddress(init.disp, data.sceneData.sceneBuffer.buffer);
//
//     //Materials Buffer
//     vmaUtils::createMaterialParamsBuffer(init, data);
//     data.materialValues.materialParamsBufferAddress = vmaUtils::getBufferDeviceAddress(init.disp, data.materialValues.materialsBuffer.buffer);
//
//     Vk_DescriptorUtils::setupDescriptors(init, data);
//
//     VkPushConstantRange pushConstantRange{};
//     pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
//     pushConstantRange.offset = 0;
//     pushConstantRange.size = sizeof(PushConstantBlock);
//     
//     VkPipelineLayoutCreateInfo pipelineLayoutInfo = Vk_DescriptorUtils::pipelineLayoutCreateInfo(&init.descriptorSetLayout, 1, pushConstantRange, 1);
//     init.disp.createPipelineLayout(&pipelineLayoutInfo, VK_NULL_HANDLE, &init.pipelineLayout);
//
//     SceneData sceneDataUBO;
//     vkUtils::prepareUBO(sceneDataUBO);
//     vmaUtils::mapPersistenData(init.vmaAllocator, data.sceneData.sceneBuffer.allocation, data.sceneData.sceneBuffer.allocationInfo, &sceneDataUBO, sizeof(SceneData));
//
//
//     size_t shaderCodeSizes[2]{};
//     char* shaderCodes[2]{};
//     
//     loadShader(std::string(SHADER_PATH) + "/mesh_shader.vert.spv", shaderCodes[0], shaderCodeSizes[0]);
//     loadShader(std::string(SHADER_PATH) + "/mesh_shader.frag.spv", shaderCodes[1], shaderCodeSizes[1]);
//     
//     data.shader_object = std::make_unique<ShaderObject>();
//     data.shader_object->create_shaders(init, shaderCodes[0], shaderCodeSizes[0], shaderCodes[1], shaderCodeSizes[1],
//         &init.descriptorSetLayout, 1, &pushConstantRange, 1);
//
//     //create depth stencil image
//     vmaUtils::getSupportedDepthStencilFormat(init.physicalDevice, &data.depthStencilImage.format);
//
//     vmaUtils::setupDepthStencil(init.disp, init.swapchain.extent, init.vmaAllocator, data.depthStencilImage);
//     
//     return 0;
// }
