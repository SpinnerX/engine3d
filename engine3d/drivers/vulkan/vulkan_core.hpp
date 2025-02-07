#pragma once
#include <type_traits>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <span>

namespace engine3d::vk{

    //! @note This just checks the result type that is invoked by the passed callable object is valid
    //! @note If the returned result is invalid then there will be a compiled error
    //! @note Used in the submit function
    template<typename Type>
    concept ReturnInvoke = std::is_same_v<std::span<VkCommandBuffer>, Type>;

    /**
     * @name CommandbufferBeginInfo
     * @note Operation to indicate when a command buffer can start to record
     * @note Creates a complelety new command buffer and indicating to start recording to that new command buffer
     * @note Use to creating a command buffer more easier
     * TODO: Depends on how this go, may change this later
    */
    VkCommandBuffer CreateCommandBufferWithBeginRecord(const VkDevice& p_LogicalDevice, const VkCommandPool p_SingleCommandPool);

    /**
     * @name BeginCommandBuffer
     * @note Enables you to begin recording the command buffer excluding actually specifying the structure VkBeginInfo
     * @note Minimize boilerplate to record command buffers
    */
    void BeginCommandBuffer(const VkCommandBuffer& p_Commands);

    /**
     * @name End Command Buffer
     * @note Operation to signal command buffer to stop recording commands
    */
    void EndCommandBuffer(const VkCommandBuffer& p_CommandBufferContext);


    /**
     * @name Submit
     * @note This is the vulkan operation to submit commands directly to the GPU
     * @note This operation is to minimize the specific boilerplate that comes with vulkan
     * @param VkQueue is the queue the user can specify to submit commands to use
     * @param p_Commands is the span of commands that will be submitted to the gpu
     * @note Typically the swapchain contains the information what queue to submit to
     * @note Swapchain also knows 
    */
    void Submit(VkQueue p_QueueToUse, std::span<VkCommandBuffer>& p_Commands);

    /**
     * @name Submit
     * @note Handles direct operation to submit commands to directly to the gpu
     * @note UCallableTask must return a valid structure that is valid to std::span<VkCommandBuffer>
     * @param VkQueue passing the queue to submit the command buffer to
     * @param UCallableTask defines a lambda function that is used to specify a large operation that requires more then just passing the command buffer itself
     * @note Usage of this is when we need to handle specific tasks just before we directly submit those commands
    */
    template<typename UCallableTask> requires ReturnInvoke<std::invoke_result_t<UCallableTask>>
    void Submit(VkQueue p_QueueToUse, const UCallableTask& p_CallableObject){
        auto commands = p_CallableObject();

        VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = static_cast<uint32_t>(commands.size()),
            .pCommandBuffers = commands.data()
        };

        vkQueueSubmit(p_QueueToUse, static_cast<uint32_t>(commands.size()), &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(p_QueueToUse);
    }

    VkImage CreateImage(VkFormat Format, uint32_t p_Width, uint32_t p_Height);
    VkImageView CreateImageView(VkImage Image, VkFormat SurfaceFormat);


    VkCommandBuffer BeginSingleTimeCommand(const VkCommandPool& p_CommandPool);
    void EndSingleTimeCommand(const VkCommandBuffer& p_CommandBuffer, const VkCommandPool& p_CommandPool);

    VkCommandPool CreatecommandPool();

    void PipelineImageBarrier(VkCommandBuffer p_CommandBuffer, VkImage Image, VkAccessFlags srcAccessmask, VkAccessFlags dstAccessMask, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkPipelineStageFlags srcStageFlags, VkPipelineStageFlags dstStageFlags, VkImageSubresourceRange subresourceRange);
};