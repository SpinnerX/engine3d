#include <drivers/vulkan/vulkan_context.hpp>
#include <drivers/vulkan/vulkan_core.hpp>
#include <vulkan/vulkan_core.h>
#include <drivers/vulkan/helper_functions.hpp>

namespace engine3d::vk{
    VkCommandBuffer CreateCommandBufferWithBeginRecord(const VkDevice& p_LogicalDevice, const VkCommandPool p_SingleCommandPool){
        VkCommandBufferAllocateInfo cmd_buffer_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = p_SingleCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkCommandBuffer command_buffer;

        vk_check(vkAllocateCommandBuffers(p_LogicalDevice, &cmd_buffer_alloc_info, &command_buffer), "vkAllocateCommandBuffers", __FILE__, __LINE__, __FUNCTION__);

        VkCommandBufferBeginInfo cmd_buffer_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vk_check(vkBeginCommandBuffer(command_buffer, &cmd_buffer_begin_info), "vkBeginCommandBuffer", __FILE__, __LINE__, __FUNCTION__);

        return command_buffer;
    }


    void BeginCommandBuffer(const VkCommandBuffer& p_CommandBuffer){
        VkCommandBufferBeginInfo cmd_buffer_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vk_check(vkBeginCommandBuffer(p_CommandBuffer, &cmd_buffer_begin_info), "vkBeginCommandBuffer", __FILE__, __LINE__, __FUNCTION__);
    }

    void EndCommandBuffer(const VkCommandBuffer& p_CommandBufferContext){
        vkEndCommandBuffer(p_CommandBufferContext);
    }

    void Submit(VkQueue p_QueueToUse, std::span<VkCommandBuffer>& p_Commands){
        VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = static_cast<uint32_t>(p_Commands.size()),
            .pCommandBuffers = p_Commands.data()
        };

        vkQueueSubmit(p_QueueToUse, static_cast<uint32_t>(p_Commands.size()), &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(p_QueueToUse);
    }

    VkImageView CreateImageView(VkImage Image, VkFormat SurfaceFormat){

        VkImageView new_image_view;
        VkImageViewCreateInfo image_view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = Image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = SurfaceFormat,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        };

        vk_check(vkCreateImageView(VulkanContext::GetDriver(), &image_view_create_info, nullptr, &new_image_view), "vkCreateImageView", __FILE__, __LINE__, __FUNCTION__);
        return new_image_view;
    }

    VkImage CreateImage(VkFormat Format, uint32_t p_Width, uint32_t p_Height){
        VkImage new_image;

        VkImageCreateInfo image_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            //! @note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
            .format = Format,
            .extent = {
                .width = p_Width,
                .height = p_Height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_LINEAR,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        vk_check(vkCreateImage(VulkanContext::GetDriver(), &image_ci, nullptr, &new_image), "vkCreateImage", __FILE__, __LINE__, __FUNCTION__);

        return new_image;
    }


    VkCommandBuffer BeginSingleTimeCommand(const VkCommandPool& p_CommandPool){
        VkCommandBufferAllocateInfo alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = p_CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VkCommandBuffer temp_command_buffer;
        vk_check(vkAllocateCommandBuffers(VulkanContext::GetDriver(), &alloc_info, &temp_command_buffer), "vkAllocateCommandBuffers", __FILE__, __LINE__, __FUNCTION__);

        VkCommandBufferBeginInfo command_buffer_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vk_check(vkBeginCommandBuffer(temp_command_buffer, &command_buffer_begin_info), "vkBeginCommandBuffer", __FILE__, __LINE__, __FUNCTION__);

        return temp_command_buffer;
    }

    void EndSingleTimeCommand(const VkCommandBuffer& p_CommandBuffer, const VkCommandPool& p_CommandPool){
        vkEndCommandBuffer(p_CommandBuffer);

        VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &p_CommandBuffer
        };

        // auto queue_graphics_idx = VulkanContext::GetPhysicalDriver().GetQueueIndices().Graphics;
        auto graphics_queue = VulkanContext::GetDriver().GetGraphicsQueue();

        vk_check(vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE), "vkQueueSubmit", __FILE__, __LINE__, __FUNCTION__);
        vk_check(vkQueueWaitIdle(graphics_queue), "vkQueueWaitIdle", __FILE__, __LINE__, __FUNCTION__);

        vkFreeCommandBuffers(VulkanContext::GetDriver(), p_CommandPool, 1, &p_CommandBuffer);
    }

    VkCommandPool CreatecommandPool(){
        VkCommandPoolCreateInfo cmd_pool_ci = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = VulkanContext::GetPhysicalDriver().GetQueueIndices().Graphics
        };

        VkCommandPool temp_command_pool;

        vk_check(vkCreateCommandPool(VulkanContext::GetDriver(), &cmd_pool_ci, nullptr, &temp_command_pool), "vkCreateCommandPool", __FILE__, __LINE__, __FUNCTION__);
        
        return temp_command_pool;
    }

    void PipelineImageBarrier(VkCommandBuffer p_CommandBuffer, VkImage Image, VkAccessFlags srcAccessmask, VkAccessFlags dstAccessMask, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkPipelineStageFlags srcStageFlags, VkPipelineStageFlags dstStageFlags, VkImageSubresourceRange subresourceRange){
        VkImageMemoryBarrier image_memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = srcAccessmask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = OldImageLayout,
            .newLayout = NewImageLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = Image,
            .subresourceRange = subresourceRange
        };

        vkCmdPipelineBarrier(p_CommandBuffer, srcStageFlags, dstStageFlags, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
    }
};