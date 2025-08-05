#include "MemoryUtils.h"
#include <iostream>
#include "../structs/Vk_RenderData.h"
#include "../structs/Vertex.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "DescriptorUtils.h"
#include "Vk_Utils.h"
#include "../structs/EngineContext.h"
#include "../vulkan/DeviceManager.h"
#include "../rendering/Renderer.h"

void utils::MemoryUtils::create_vma_allocator(vulkan::DeviceManager& device_manager)
{
    PFN_vkGetInstanceProcAddr fnGetInstanceProcAddr = (vkGetInstanceProcAddr);
    PFN_vkGetDeviceProcAddr fnGetDeviceProcAddr = (vkGetDeviceProcAddr);
    
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = fnGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = fnGetDeviceProcAddr;

    //TODO: Change later Needed to run with NVIDIA NSights
    vulkanFunctions.vkGetBufferMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2KHR>(vkGetInstanceProcAddr(device_manager.get_instance(), "vkGetBufferMemoryRequirements2KHR"));
    vulkanFunctions.vkGetImageMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetImageMemoryRequirements2KHR>(vkGetInstanceProcAddr(device_manager.get_instance(), "vkGetImageMemoryRequirements2KHR"));
    vulkanFunctions.vkGetBufferMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2KHR>(vkGetInstanceProcAddr(device_manager.get_instance(), "vkGetBufferMemoryRequirements2KHR"));
    vulkanFunctions.vkBindBufferMemory2KHR = reinterpret_cast<PFN_vkBindBufferMemory2KHR>(vkGetInstanceProcAddr(device_manager.get_instance(), "vkBindBufferMemory2KHR"));
    vulkanFunctions.vkBindImageMemory2KHR = reinterpret_cast<PFN_vkBindImageMemory2KHR>(vkGetInstanceProcAddr(device_manager.get_instance(), "vkBindImageMemory2KHR"));
    
    static VmaAllocatorCreateInfo allocatorInfo;

    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorInfo.physicalDevice = device_manager.get_device().physical_device;
    allocatorInfo.instance = device_manager.get_instance();
    allocatorInfo.device = device_manager.get_device();
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;

    VmaAllocator allocator;
    
    VkResult result = vmaCreateAllocator(&allocatorInfo, &allocator);
    if (result != VK_SUCCESS)
    {
        std::cerr << "Failed to create VMA allocator: " << result << std::endl;
        throw std::runtime_error("Failed to create VMA allocator!");
    }

    device_manager.set_vma_allocator(allocator);

    std::cout << "VMA allocator created successfully." << std::endl;
}

void utils::MemoryUtils::create_buffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
                               VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags vmaAllocationFlags, GPU_Buffer& outBuffer) 
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = memoryUsage;  
    allocCreateInfo.flags = vmaAllocationFlags;

    if (VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &outBuffer.buffer, &outBuffer.allocation, &outBuffer.allocation_info); result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer with VMA!");
    }
}

VkResult utils::MemoryUtils::map_persistent_data(VmaAllocator vmaAllocator, VmaAllocation allocation, const VmaAllocationInfo& allocationInfo, const void* data, VkDeviceSize bufferSize)
{
    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(vmaAllocator, allocation, &memPropFlags);
    
    if(memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        memcpy(allocationInfo.pMappedData, data, bufferSize);
        VkResult result = vmaFlushAllocation(vmaAllocator, allocation, 0, VK_WHOLE_SIZE);
        return result;
    }
    
     return VK_ERROR_UNKNOWN;
}

VkDeviceAddress utils::MemoryUtils::get_buffer_device_address(const vkb::DispatchTable& disp, VkBuffer buffer)
{
    VkBufferDeviceAddressInfoEXT bufferDeviceAI{};
    bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAI.buffer = buffer;

    PFN_vkGetDeviceProcAddr fnGetDeviceProcAddr = (vkGetDeviceProcAddr);
    PFN_vkGetBufferDeviceAddressEXT fnGetBufferDeviceAddressEXT = reinterpret_cast<PFN_vkGetBufferDeviceAddressEXT>(vkGetDeviceProcAddr(disp.device, "vkGetBufferDeviceAddressKHR"));
    
    return fnGetBufferDeviceAddressEXT(disp.device, &bufferDeviceAI);
}


void utils::MemoryUtils::copy_buffer(vkb::DispatchTable disp, VkQueue queue, VkCommandPool command_pool, VkBuffer srcBuffer,
                          VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    disp.allocateCommandBuffers(&allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    disp.beginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    disp.cmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    disp.endCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    disp.queueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    disp.queueWaitIdle(queue);

    disp.freeCommandBuffers(command_pool, 1, &commandBuffer);
}

void utils::MemoryUtils::create_vertex_and_index_buffers(
    EngineContext& engine_context, const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices, GPU_Buffer& out_vertex_buffer, GPU_Buffer& out_index_buffer)
{
    auto device_manager = engine_context.device_manager.get();
    auto renderer = engine_context.renderer.get();
    
    VkDeviceSize vertexBufferSize = sizeof(Vertex) * vertices.size();
    VkDeviceSize indexBufferSize = sizeof(uint32_t) * indices.size();

    // Create Staging Buffer for Vertices using VMA
    GPU_Buffer staging_vertex_buffer;
    create_buffer(device_manager->get_allocator(), vertexBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VMA_MEMORY_USAGE_AUTO,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |  VMA_ALLOCATION_CREATE_MAPPED_BIT,
                    staging_vertex_buffer);

    // Copy Vertex Data to Staging Buffer using VMA mapping
    assert(vertices.size() != 0, "Vertex Data is empty!");
    
    void* data;
    vmaMapMemory(device_manager->get_allocator(), staging_vertex_buffer.allocation, &data);
    memcpy(data, vertices.data(),  vertexBufferSize);
    vmaUnmapMemory(engine_context.device_manager->get_allocator(), staging_vertex_buffer.allocation);

    // Create Vertex Buffer (Device Local) using VMA
    create_buffer(device_manager->get_allocator(), vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,out_vertex_buffer);

    // Copy from Staging Vertex Buffer to Device Local Vertex Buffer
    copy_buffer(engine_context.dispatch_table, device_manager->get_graphics_queue(), renderer->get_command_pool(), staging_vertex_buffer.buffer, out_vertex_buffer.buffer, vertexBufferSize);
    set_vulkan_object_Name(engine_context.dispatch_table, (uint64_t) out_vertex_buffer.buffer, VK_OBJECT_TYPE_BUFFER, "Vertex Buffer");

    // Clean up Staging Vertex Buffer using VMA
    vmaDestroyBuffer(device_manager->get_allocator(), staging_vertex_buffer.buffer, staging_vertex_buffer.allocation);

    // Create Staging Buffer for Indices using VMA
    GPU_Buffer staging_index_buffer;
    create_buffer(device_manager->get_allocator(), indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |  VMA_ALLOCATION_CREATE_MAPPED_BIT,
                    staging_index_buffer);

    // Copy Index Data to Staging Buffer using VMA mapping
    vmaMapMemory(device_manager->get_allocator(), staging_index_buffer.allocation, &data);
    memcpy(data, indices.data(), (size_t) indexBufferSize);
    vmaUnmapMemory(device_manager->get_allocator(), staging_index_buffer.allocation);

    // Create Index Buffer (Device Local) using VMA
    create_buffer(device_manager->get_allocator(), indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_AUTO,
                    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, out_index_buffer);
    
    utils::set_vulkan_object_Name(engine_context.dispatch_table, (uint64_t) out_index_buffer.buffer, VK_OBJECT_TYPE_BUFFER, "Index Buffer");

    // Copy from Staging Index Buffer to Device Local Index Buffer
    copy_buffer(engine_context.dispatch_table, device_manager->get_graphics_queue(), renderer->get_command_pool(), staging_index_buffer.buffer, out_index_buffer.buffer, indexBufferSize);

    // Clean up Staging Index Buffer using VMA
    vmaDestroyBuffer(device_manager->get_allocator(), staging_index_buffer.buffer, staging_index_buffer.allocation);
}

void utils::MemoryUtils::createMaterialParamsBuffer(const vulkan::DeviceManager& device_manager,
                                                    const std::unordered_map<uint32_t, MaterialParams>& material_params,
                                                    GPU_Buffer& out_material_params_buffer)
{
    uint32_t maxMaterialIndex = 0;
    if (!material_params.empty())
    {
        // Find the element with the maximum key (material index)
        auto max_it = std::ranges::max_element(material_params,
                                               [](const auto& a, const auto& b)
                                               {
                                                   return a.first < b.first;
                                               });
        maxMaterialIndex = max_it->first;
    }

    // The vector size needs to be max_index + 1 to accommodate all indices from 0 to max_index
    size_t materialCount = maxMaterialIndex + 1;
    std::vector<MaterialParams> materialData(materialCount);

    // Populate the vector using the map, placing each material at its correct index
    for (const auto& pair : material_params)
    {
        if (pair.first < materialCount)
        {
            materialData[pair.first] = pair.second;
        }
        else
        {
            std::cerr << "Warning: Material index " << pair.first << " is out of expected range (" << materialCount << ")" << std::endl;
        }
    }

    VkDeviceSize bufferSize = sizeof(MaterialParams) * materialData.size();
    //allocate_buffer_with_mapped_access(device_manager.get_allocator(), bufferSize, out_material_params_buffer);
    
    map_persistent_data(device_manager.get_allocator(), out_material_params_buffer.allocation, out_material_params_buffer.allocation_info, materialData.data(), bufferSize);
}

void utils::MemoryUtils::allocate_buffer_with_mapped_access(VmaAllocator allocator, VkDeviceSize size, GPU_Buffer& buffer)
{
    create_buffer(allocator, size,
                     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,  
                     VMA_MEMORY_USAGE_AUTO,
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                     VMA_ALLOCATION_CREATE_MAPPED_BIT, buffer);
}

void utils::MemoryUtils::allocate_buffer_with_readback_access(VmaAllocator allocator, VkDeviceSize size,
    GPU_Buffer& buffer)
{
    create_buffer(allocator, size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
        VMA_ALLOCATION_CREATE_MAPPED_BIT, buffer);
}
