#include "Vk_DynamicRendering.h"

VkRenderingInfoKHR Vk_DynamicRendering::rendering_info(VkRect2D render_area, uint32_t color_attachment_count,
    const VkRenderingAttachmentInfoKHR* pColorAttachments, VkRenderingFlagsKHR flags)
{
    VkRenderingInfoKHR rendering_info   = {};
    rendering_info.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    rendering_info.pNext                = VK_NULL_HANDLE;
    rendering_info.flags                = flags;
    rendering_info.renderArea           = render_area;
    rendering_info.layerCount           = 1;
    rendering_info.viewMask             = 0;
    rendering_info.colorAttachmentCount = color_attachment_count;
    rendering_info.pColorAttachments    = pColorAttachments;
    rendering_info.pDepthAttachment     = VK_NULL_HANDLE;
    rendering_info.pStencilAttachment   = VK_NULL_HANDLE;
    return rendering_info;
}

void Vk_DynamicRendering::image_layout_transition(VkCommandBuffer command_buffer, VkImage image,
    VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask, VkAccessFlags src_access_mask,
    VkAccessFlags dst_access_mask, VkImageLayout old_layout, VkImageLayout new_layout,
    const VkImageSubresourceRange& subresource_range)
{
    // Define an image memory barrier
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;       // Previous image layout
    barrier.newLayout = new_layout;       // Target image layout
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;                // Target image
    barrier.subresourceRange = subresource_range; // Range of image subresources

    // Set source and destination access masks
    barrier.srcAccessMask = src_access_mask; // Access mask for the previous layout
    barrier.dstAccessMask = dst_access_mask; // Access mask for the target layout

    // Record the pipeline barrier into the command buffer
    vkCmdPipelineBarrier(
        command_buffer,  // Command buffer
        src_stage_mask,  // Source pipeline stage
        dst_stage_mask,  // Destination pipeline stage
        0,               // Dependency flags (0 for none)
        0, nullptr,      // Memory barriers (none in this example)
        0, nullptr,      // Buffer memory barriers (none in this example)
        1, &barrier      // Image memory barriers
    );
}
