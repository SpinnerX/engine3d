#include "drivers/vulkan/helper_functions.hpp"
#include "drivers/vulkan/vulkan_context.hpp"
#include "engine_logger.hpp"
#include <drivers/vulkan/buffer_tutorial.hpp>
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace engine3d::vk{
    static uint64_t GetAlignment(uint64_t p_InstanceSize, uint64_t p_MinOffsetAlignment){
        if(p_MinOffsetAlignment > 0){
            return (p_InstanceSize + p_MinOffsetAlignment - 1) & ~(p_MinOffsetAlignment - 1);
        }
        
        return p_InstanceSize;
    }

    BufferTutorial::BufferTutorial(uint64_t p_InstanceSize, uint32_t p_InstanceCount, VkBufferUsageFlags p_UsageFlags, VkMemoryPropertyFlags p_MemoryPropertyFlags, uint64_t p_MinOffsetAlignment) :
                                  m_InstanceCount(p_InstanceCount),
                                  m_InstanceSize(p_InstanceSize),
                                  m_UsageFlags(p_UsageFlags),
                                  m_MemoryPropertyFlags(p_MemoryPropertyFlags){
        m_AlignmentSize = GetAlignment(m_InstanceSize, p_MinOffsetAlignment);
        m_BufferSize = m_AlignmentSize * m_InstanceCount;

        //! @note Creating buffer
        // device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
        VkBufferCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = m_BufferSize,
            .usage = m_UsageFlags,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        
        
        // Creating buffer before getting memory requirements
        vk_check(vkCreateBuffer(VulkanContext::GetDriver(), &create_info, nullptr, &m_BufferHandler), "vkCreateBuffer", __FILE__, __LINE__, __FUNCTION__);

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(VulkanContext::GetDriver(), m_BufferHandler, &memory_requirements);

        VkMemoryAllocateInfo alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = VulkanContext::GetPhysicalDriver().SearchMemoryType(memory_requirements.memoryTypeBits, p_MemoryPropertyFlags)
        };

        vk_check(vkAllocateMemory(VulkanContext::GetDriver(), &alloc_info, nullptr, &m_BufferDeviceMemory), "vkAllocateMemory", __FILE__, __LINE__, __FUNCTION__);
    }

    void BufferTutorial::Map(VkDeviceSize p_Size, uint64_t p_Offset){
        vk_check(vkMapMemory(VulkanContext::GetDriver(), m_BufferDeviceMemory, p_Offset, p_Size, 0, &m_MappedData), "vkMapMemory", __FILE__, __LINE__, __FUNCTION__);
    }

    void BufferTutorial::Unmap(){
        if(m_BufferDeviceMemory){
            vkUnmapMemory(VulkanContext::GetDriver(), m_BufferDeviceMemory);
        }
    }

    void BufferTutorial::WriteToBuffer(void* p_Data, uint64_t p_Size, uint64_t p_Offset){
        if(p_Size == VK_WHOLE_SIZE){
            memcpy(m_MappedData, p_Data, m_BufferSize);
        }
        else{
            char* mem_offset = (char*)m_MappedData;
            mem_offset += p_Offset;
            memcpy(mem_offset, p_Data, p_Size);
        }
    }

    void BufferTutorial::Flush(uint64_t p_Size, uint64_t offset){
        VkMappedMemoryRange mapped_mem_range = {
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .pNext = nullptr,
            .memory = m_BufferDeviceMemory,
            .offset = offset,
            .size = p_Size
        };

        vk_check(vkFlushMappedMemoryRanges(VulkanContext::GetDriver(), 1, &mapped_mem_range), "vkFlushMappedMemoryRanges", __FILE__, __LINE__, __FUNCTION__);
    }
    
    VkDescriptorBufferInfo BufferTutorial::DescriptorInfo(uint64_t p_Size, uint64_t p_Offset){
        return VkDescriptorBufferInfo{
            .buffer = m_BufferHandler,
            .offset = p_Offset,
            .range = p_Size
        };
    }

    void BufferTutorial::Invalidate(uint64_t p_Size, uint64_t p_Offset){
        VkMappedMemoryRange mapped_mem_range = {
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = m_BufferDeviceMemory,
            .offset = p_Offset,
            .size = p_Size
        };

        vk_check(vkInvalidateMappedMemoryRanges(VulkanContext::GetDriver(), 1, &mapped_mem_range), "vkInvalidateMappedMemoryRanges", __FILE__, __LINE__, __FUNCTION__);
    }

    void BufferTutorial::WriteToIndex(void* data, int idx){
        WriteToBuffer(data, m_InstanceSize, idx * m_AlignmentSize);
    }

    void BufferTutorial::FlushIndex(int index){
        Flush(m_AlignmentSize, index * m_AlignmentSize);
    }

    VkDescriptorBufferInfo BufferTutorial::DescriptorInfoForIndex(int index){
        return DescriptorInfo(m_AlignmentSize, index * m_AlignmentSize);
    }

    void BufferTutorial::InvalidateIndex(int index){
        Invalidate(m_AlignmentSize, index * m_AlignmentSize);
    }
};