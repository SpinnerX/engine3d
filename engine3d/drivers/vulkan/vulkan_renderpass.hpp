#pragma once
#include <drivers/render_pass.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace engine3d::vk{
    class VulkanRenderPass : public RenderPass{
    public:
        VulkanRenderPass(const RenderPassProperties& p_Properties);
        
    protected:
        void BeginRenderPass() override;
        void EndRenderPass() override;

    private:
        VkRenderPass m_RenderPassHandler;
    };
};