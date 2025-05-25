#pragma once
#include <vulkan_core.h>

class Vk_DynamicRendering
{
public:
    static VkRenderingInfoKHR rendering_info(VkRect2D render_area = {},
                                      uint32_t color_attachment_count = 0,
                                      const VkRenderingAttachmentInfoKHR *pColorAttachments = VK_NULL_HANDLE,
                                      VkRenderingFlagsKHR flags = 0);
    
    static void image_layout_transition(VkCommandBuffer command_buffer,
                                    VkImage image,
                                    VkPipelineStageFlags src_stage_mask,
                                    VkPipelineStageFlags dst_stage_mask,
                                    VkAccessFlags src_access_mask,
                                    VkAccessFlags dst_access_mask,
                                    VkImageLayout old_layout,
                                    VkImageLayout new_layout,
                                    const VkImageSubresourceRange &subresource_range);
};
