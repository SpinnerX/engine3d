#pragma once

#include <vulkan/vulkan_core.h>
namespace engine3d::vk{
    /**
     * @name BufferTutorial
     * @note Buffer from the tutorial
    */
    class BufferTutorial{
    public:
        BufferTutorial() = default;
        BufferTutorial(uint64_t p_InstanceSize, uint32_t p_InstanceCount, VkBufferUsageFlags p_UsageFlags, VkMemoryPropertyFlags p_MemoryPropertyFlags, uint64_t p_MinOffsetAlignment=1);

        void Map(VkDeviceSize p_Size = VK_WHOLE_SIZE, uint64_t p_Offset = 0);
        void Unmap();

        void WriteToBuffer(void* data, uint64_t p_Size = VK_WHOLE_SIZE, uint64_t p_Offset = 0);
        void Flush(uint64_t p_Size = VK_WHOLE_SIZE, uint64_t offset = 0);
        VkDescriptorBufferInfo DescriptorInfo(uint64_t p_Size = VK_WHOLE_SIZE, uint64_t p_Offset = 0);
        void Invalidate(uint64_t p_Size = VK_WHOLE_SIZE, uint64_t p_Offset = 0);

        void WriteToIndex(void* data, int idx);

        void FlushIndex(int index);
        VkDescriptorBufferInfo DescriptorInfoForIndex(int index);
        void InvalidateIndex(int index);

        VkBuffer GetBufferInstance() { return m_BufferHandler; }

        void* GetMappedData() const { return m_MappedData; }
        uint32_t GetInstanceCount() const { return m_InstanceCount; }

        uint64_t GetInstanceSize() const { return m_InstanceSize; }
        uint64_t GetAlignmentSize() const { return m_AlignmentSize; }

        VkBufferUsageFlags GetUsageFlags() const { return m_UsageFlags; }




    private:
        VkBuffer m_BufferHandler = VK_NULL_HANDLE;
        VkDeviceMemory m_BufferDeviceMemory = VK_NULL_HANDLE;
        void* m_MappedData = nullptr;
        uint64_t m_BufferSize;
        uint32_t m_InstanceCount;
        uint64_t m_InstanceSize;
        uint64_t m_AlignmentSize;
        VkBufferUsageFlags m_UsageFlags;
        VkMemoryPropertyFlags m_MemoryPropertyFlags;


    };
};