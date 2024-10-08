#pragma once
#include "internal/VulkanCpp/VulkanLogicalDevice.h"

namespace engine3d::vk{
    class VulkanDevice{
    public:
        static void InitializeVulkanDevice();
        static void CleanupVulkanDevices();


        static VulkanPhysicalDevice& GetPhysicalDevice();
        static VulkanLogicalDevice& GetLogicalDevice();
    };
};