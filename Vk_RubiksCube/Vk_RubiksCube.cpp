
#include "Vk_RubiksCube.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <thread>

#include "Config.h"
#include "materials/ShaderObject.h"
#include "rendering/Vk_DynamicRendering.h"
#include "utils/ModelUtils.h"
#include "utils/VMA_MemoryUtils.h"

GLFWwindow* create_window_glfw(const char* window_name, bool resize)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (!resize) glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return glfwCreateWindow(1024, 1024, window_name, NULL, NULL);
}

void destroy_window_glfw(GLFWwindow* window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

VkSurfaceKHR create_surface_glfw(VkInstance instance, GLFWwindow* window, VkAllocationCallbacks* allocator)
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult err = glfwCreateWindowSurface(instance, window, allocator, &surface);
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

int device_initialization(Init& init)
{
    init.window = create_window_glfw("Rubik's Cube", true);

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
        return -1;
    }
    init.instance = instance_ret.value();

    init.inst_disp = init.instance.make_table();

    init.surface = create_surface_glfw(init.instance, init.window);

    VkPhysicalDeviceFeatures features = {};
    features.geometryShader = VK_FALSE;
    features.tessellationShader = VK_FALSE;

    vkb::PhysicalDeviceSelector phys_device_selector(init.instance);
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
        .add_required_extension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)
        //.add_required_extension(VK_KHR_MAINTENANCE_6_EXTENSION_NAME)
        .set_required_features(features)
        .set_surface(init.surface)
        .select();

    auto dynamic_rendering_features = Vk_DynamicRendering::create_dynamic_rendering_features();
    auto shader_object_features = ShaderObject::create_shader_object_features();
    auto device_memory_features = vmaUtils::create_physical_device_buffer_address();
    
    if (!phys_device_ret)
    {
        std::cout << phys_device_ret.error().message() << "\n";
        return -1;
    }
    const vkb::PhysicalDevice& physical_device = phys_device_ret.value();

    vkb::DeviceBuilder device_builder{ physical_device };
    auto device_ret = device_builder
        .add_pNext(&dynamic_rendering_features)
        .add_pNext(&shader_object_features)
        .add_pNext(&device_memory_features)
        .build();
    
    if (!device_ret)
    {
        std::cout << device_ret.error().message() << "\n";
        return -1;
    }
    init.device = device_ret.value();

    init.disp = init.device.make_table();

    return 0;
}

int create_swapchain(Init& init)
{
    vkb::SwapchainBuilder swapchain_builder{ init.device };
    auto swap_ret = swapchain_builder.set_old_swapchain(init.swapchain).build();
    if (!swap_ret)
    {
        std::cout << swap_ret.error().message() << " " << swap_ret.vk_result() << "\n";
        return -1;
    }
    vkb::destroy_swapchain(init.swapchain);
    init.swapchain = swap_ret.value();
    return 0;
}

int get_queues(Init& init, RenderData& data)
{
    auto gq = init.device.get_queue(vkb::QueueType::graphics);
    if (!gq.has_value())
    {
        std::cout << "failed to get graphics queue: " << gq.error().message() << "\n";
        return -1;
    }
    data.graphics_queue = gq.value();

    auto pq = init.device.get_queue(vkb::QueueType::present);
    if (!pq.has_value()) {
        std::cout << "failed to get present queue: " << pq.error().message() << "\n";
        return -1;
    }
    data.present_queue = pq.value();
    return 0;
}

void loadShader(std::string filename, char* &code, size_t &size)
{
    std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);
    if (is.is_open())
    {
        size = is.tellg();
        is.seekg(0, std::ios::beg);
        code = new char[size];
        is.read(code, size);
        is.close();
        assert(size > 0);
    }
    else
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }
}

VkShaderModule createShaderModule(Init& init, const std::vector<char>& code)
{
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (init.disp.createShaderModule(&create_info, nullptr, &shaderModule) != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
        // failed to create shader module
    }

    return shaderModule;
}

std::vector<uint32_t> convert_char_to_uint32(const std::vector<char>& char_vec)
{
    // Ensure the size of the byte vector is a multiple of 4 (size of uint32_t)
    if (char_vec.size() % sizeof(uint32_t) != 0) {
        throw std::runtime_error("Raw SPIR-V size is not a multiple of 4 bytes!");
    }

    // Calculate the number of uint32_t elements
    size_t uint32_count = char_vec.size() / sizeof(uint32_t);

    // Create the destination vector
    std::vector<uint32_t> uint32_vec(uint32_count);

    // Reinterpret the data. Using memcpy is generally safer than a direct reinterpret_cast
    // for potentially unaligned data, although vector data is usually aligned.
    std::memcpy(uint32_vec.data(), char_vec.data(), char_vec.size());

    return uint32_vec;
}


int create_graphics_pipeline(Init& init, RenderData& data) 
{
    size_t shaderCodeSizes[2]{};
    char* shaderCodes[2]{};
    
    loadShader(std::string(SHADER_PATH) + "/mesh_shader.vert.spv", shaderCodes[0], shaderCodeSizes[0]);
    loadShader(std::string(SHADER_PATH) + "/mesh_shader.frag.spv", shaderCodes[1], shaderCodeSizes[1]);
    
    data.shader_object = std::make_unique<ShaderObject>();
    data.shader_object->create_shaders(init.disp, shaderCodes[0], shaderCodeSizes[0], shaderCodes[1], shaderCodeSizes[1]);
    
    return 0;
}

int create_command_pool(Init& init, RenderData& data) 
{
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = init.device.get_queue_index(vkb::QueueType::graphics).value();
    if (init.disp.createCommandPool(&pool_info, nullptr, &data.command_pool) != VK_SUCCESS)
    {
        std::cout << "failed to create command pool\n";
        return -1; // failed to create command pool
    }
    return 0;
}

int create_command_buffers(Init& init, RenderData& data)
{
    data.command_buffers.resize(init.swapchain.image_count);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = data.command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(data.command_buffers.size());

    if (init.disp.allocateCommandBuffers(&allocInfo, data.command_buffers.data()) != VK_SUCCESS)
    {
        // failed to allocate command buffers;
        return -1;
    }

    for (size_t i = 0; i < data.command_buffers.size(); i++)
    {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (init.disp.beginCommandBuffer(data.command_buffers[i], &begin_info) != VK_SUCCESS)
        {
            return -1;
            // failed to begin recording command buffer
        }

        VkImageSubresourceRange range{};
        range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel   = 0;
        range.levelCount     = VK_REMAINING_MIP_LEVELS;
        range.baseArrayLayer = 0;
        range.layerCount     = VK_REMAINING_ARRAY_LAYERS;

        Vk_DynamicRendering::image_layout_transition(data.command_buffers[i],
                                         init.swapchain.get_images().value()[i],
                                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                         0,
                                         VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                         VK_IMAGE_LAYOUT_UNDEFINED,
                                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                         range);
        
        VkRenderingAttachmentInfoKHR color_attachment_info = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color_attachment_info.pNext = VK_NULL_HANDLE;
        color_attachment_info.imageView                    = init.swapchain.get_image_views().value()[i];        // color_attachment.image_view;
        color_attachment_info.imageLayout                  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment_info.resolveMode                  = VK_RESOLVE_MODE_NONE;
        color_attachment_info.loadOp                       = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment_info.storeOp                      = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment_info.clearValue                   = {0.5f, 0.2f, 0.3f, 1.0f}; ;

        auto render_area             = VkRect2D{VkOffset2D{}, VkExtent2D{init.swapchain.extent.width, init.swapchain.extent.height}};
        auto render_info             = Vk_DynamicRendering::rendering_info(render_area, 1, &color_attachment_info);

        render_info.layerCount       = 1;
        render_info.pColorAttachments = &color_attachment_info;
        render_info.pDepthAttachment = VK_NULL_HANDLE;
        render_info.pStencilAttachment = VK_NULL_HANDLE;
        
        init.disp.cmdBeginRenderingKHR(data.command_buffers[i], &render_info);
        
        data.shader_object->set_initial_state(init, data.command_buffers[i]);
        
        //Draw Stuff goes here
        //init.disp.cmdDraw(data.command_buffers[i], 3, 1, 0, 0);

        //Bind buffers
        VkBuffer vertexBuffers[] = {data.vertexBuffer};
        VkDeviceSize offsets[] = {0};
        init.disp.cmdBindVertexBuffers(data.command_buffers[i], 0, 1, vertexBuffers, offsets);
        init.disp.cmdBindIndexBuffer(data.command_buffers[i], data.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        data.shader_object->bind_material_shader(init.disp, data.command_buffers[i]);
        
        
        //init.disp.cmdDraw(data.command_buffers[i], static_cast<uint32_t>(data.outVertices.size()), 1, 0, 0);
        
        // Issue the draw call using the index buffer
        init.disp.cmdDrawIndexed(data.command_buffers[i], static_cast<uint32_t>(data.outIndices.size()), 1, 0, 0,0);
        init.disp.cmdEndRenderingKHR(data.command_buffers[i]);

        Vk_DynamicRendering::image_layout_transition
        (
            data.command_buffers[i],                            // Command buffer
            init.swapchain.get_images().value()[i],               // Swapchain image
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // Source pipeline stage
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,     // Destination pipeline stage
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,     // Source access mask
            0,                                        // Destination access mask
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // Old layout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,          // New layout
            range);

        if (init.disp.endCommandBuffer(data.command_buffers[i]) != VK_SUCCESS)
        {
            std::cout << "failed to record command buffer\n";
            return -1; // failed to record command buffer!
        }
    }
    return 0;
}

int create_sync_objects(Init& init, RenderData& data)
{
    data.available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    data.finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
    data.in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    data.image_in_flight.resize(init.swapchain.image_count, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (init.disp.createSemaphore(&semaphore_info, nullptr, &data.available_semaphores[i]) != VK_SUCCESS ||
            init.disp.createSemaphore(&semaphore_info, nullptr, &data.finished_semaphore[i]) != VK_SUCCESS ||
            init.disp.createFence(&fence_info, nullptr, &data.in_flight_fences[i]) != VK_SUCCESS)
        {
            std::cout << "failed to create sync objects\n";
            return -1; // failed to create synchronization objects for a frame
        }
    }
    return 0;
}

int recreate_swapchain(Init& init, RenderData& data)
{
    init.disp.deviceWaitIdle();

    init.disp.destroyCommandPool(data.command_pool, nullptr);

    init.swapchain.destroy_image_views(data.swapchain_image_views);
    if (0 != create_swapchain(init)) return -1;
    if (0 != create_command_pool(init, data)) return -1;
    if (0 != create_command_buffers(init, data)) return -1;
    return 0;
}

int draw_frame(Init& init, RenderData& data)
{
    init.disp.waitForFences(1, &data.in_flight_fences[data.current_frame], VK_TRUE, UINT64_MAX);
    uint32_t image_index = 0;
    VkResult result = init.disp.acquireNextImageKHR(
        init.swapchain, UINT64_MAX, data.available_semaphores[data.current_frame], VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        return recreate_swapchain(init, data);
    } 
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        std::cout << "failed to acquire swapchain image. Error " << result << "\n";
        return -1;
    }

    if (data.image_in_flight[image_index] != VK_NULL_HANDLE) 
    {
        init.disp.waitForFences(1, &data.image_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }
    data.image_in_flight[image_index] = data.in_flight_fences[data.current_frame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = { data.available_semaphores[data.current_frame] };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = wait_semaphores;
    submitInfo.pWaitDstStageMask = wait_stages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &data.command_buffers[image_index];

    VkSemaphore signal_semaphores[] = { data.finished_semaphore[data.current_frame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signal_semaphores;

    init.disp.resetFences(1, &data.in_flight_fences[data.current_frame]);
    if (init.disp.queueSubmit(data.graphics_queue, 1, &submitInfo, data.in_flight_fences[data.current_frame]) != VK_SUCCESS) 
    {
        std::cout << "failed to submit draw command buffer\n";
        return -1; //"failed to submit draw command buffer
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapChains[] = { init.swapchain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapChains;

    present_info.pImageIndices = &image_index;

    result = init.disp.queuePresentKHR(data.present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        return recreate_swapchain(init, data);
    }
    else if (result != VK_SUCCESS)
    {
        std::cout << "failed to present swapchain image\n";
        return -1;
    }

    data.current_frame = (data.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    return 0;
}

void cleanup(Init& init, RenderData& data)
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        init.disp.destroySemaphore(data.finished_semaphore[i], nullptr);
        init.disp.destroySemaphore(data.available_semaphores[i], nullptr);
        init.disp.destroyFence(data.in_flight_fences[i], nullptr);
    }

    init.disp.destroyCommandPool(data.command_pool, nullptr);

    init.disp.destroyPipeline(data.graphics_pipeline, nullptr);
    init.disp.destroyPipelineLayout(data.pipeline_layout, nullptr);

    init.swapchain.destroy_image_views(data.swapchain_image_views);

    vkb::destroy_swapchain(init.swapchain);
    vkb::destroy_device(init.device);
    vkb::destroy_surface(init.instance, init.surface);
    vkb::destroy_instance(init.instance);
    destroy_window_glfw(init.window);
}

void loadModel(RenderData& data)
{
    vkUtils::loadModel(std::string(RESOURCE_PATH) + "/models/rubiks_cube/rubiks_cube.obj", data.outVertices, data.outIndices);
    
    std::cout << "Vertices: " << data.outVertices.size() << std::endl;
    std::cout << "Indices: " << data.outIndices.size() << std::endl;


    // data.outVertices = {
    //     // Front face (Z = +0.5)
    //     {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 0: Bottom-left-front
    //     {{ 0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 1: Bottom-right-front
    //     {{ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 2: Top-right-front
    //     {{-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 3: Top-left-front
    //
    //     // Back face (Z = -0.5)
    //     {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 4: Bottom-left-back
    //     {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 5: Bottom-right-back
    //     {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 6: Top-right-back
    //     {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}  // 7: Top-left-back
    // };
    //
    // // Define index data (36 indices for 12 triangles)
    // // This defines the triangles using the vertices above.
    // // Assuming uint32_t for indices, change to uint16_t if you use that.
    // data.outIndices = {
    //     // Front face
    //     0, 1, 2, 0, 2, 3,
    //     // Back face
    //     4, 6, 5, 4, 7, 6,
    //     // Top face
    //     3, 2, 6, 3, 6, 7,
    //     // Bottom face
    //     0, 4, 5, 0, 5, 1,
    //     // Right face
    //     1, 5, 6, 1, 6, 2,
    //     // Left face
    //     4, 0, 3, 4, 3, 7
    // };
}

int main()
{

    //std::this_thread::sleep_for(std::chrono::seconds(10));
    
    Init init;
    RenderData render_data;

    if (0 != device_initialization(init)) return -1;
    if (0 != create_swapchain(init)) return -1;
    if (0 != get_queues(init, render_data)) return -1;
    if (0 != create_graphics_pipeline(init, render_data)) return -1;
    if (0 != create_command_pool(init, render_data)) return -1;

    //load model
    loadModel(render_data);
    vmaUtils::createVmaAllocator(init);
    vmaUtils::createVertexAndIndexBuffersVMA(init.vmaAllocator, init.disp, render_data.graphics_queue, render_data.command_pool, render_data, render_data.outVertices, render_data.outIndices);
    
    
    if (0 != create_command_buffers(init, render_data)) return -1;
    if (0 != create_sync_objects(init, render_data)) return -1;

    while (!glfwWindowShouldClose(init.window))
    {
        glfwPollEvents();
        int res = draw_frame(init, render_data);
        if (res != 0)
        {
            std::cout << "failed to draw frame \n";
            return -1;
        }
    }
    
    init.disp.deviceWaitIdle();

    cleanup(init, render_data);
    return 0;
}
