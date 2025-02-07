#pragma once

#include <vulkan/vulkan_core.h>
namespace engine3d::vk{
    
    void BeginCommandBuffer(VkCommandBuffer* p_CommandBuffer);
    void EndCommandBuffer(VkCommandBuffer* p_CommandBuffer);
};