#include "application_instance.hpp"
#include <drivers/render_pass.hpp>
#include <drivers/vulkan/vulkan_renderpass.hpp>

namespace engine3d{
    Ref<RenderPass> RenderPass::Create(const RenderPassProperties& properties){
        switch (ApplicationInstance::CurrentAPI()){
        case API::VULKAN: CreateRef<vk::VulkanRenderPass>(properties);
        default: break;
        }

        return nullptr;
    }
};