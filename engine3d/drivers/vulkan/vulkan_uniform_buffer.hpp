#pragma once
#include <vulkan/vulkan_core.h>

namespace engine3d::vk{
    
    /**
     * @name Vulkan Uniform Buffer Object
     * @note Defines how we create different uniform buffers
     * @note Read-only for shaders
    */
    class VulkanUniformBuffer{
    public:
        VulkanUniformBuffer() = default;
        /**
         * @param Size VkDeviceSize of the buffer
         * @param InstanceCount count of the instances corresponding to this buffer
        */
        VulkanUniformBuffer(uint32_t p_BufferSize, uint32_t p_InstanceCount);

        void AllocateUniforms();
        void MapWrite(uint32_t p_size, uint32_t p_Offset, const void* p_Src);

    private:
        VkBuffer m_BufferObject;
        VkDeviceMemory m_BufferMemory;
    };
};