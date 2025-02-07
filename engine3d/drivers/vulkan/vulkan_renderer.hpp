#pragma once
#include <string>
#include <vulkan/vulkan_core.h>
#include <drivers/renderer_backend.hpp>

namespace engine3d::vk{
    /**
     * @name VulkanRenderer
     * @note vulkan-backend specific renderer
     * @note renderer that is implemented with the vulkan's API
     * @note Provide an API on directly submitting tasks to the vulkan API that gets send directly to the GPU
    */
    class VulkanRenderer : public RendererContext{
    public:
        VulkanRenderer(const std::string& Tag);
        //! @note Implementation-details for sending draw calls.

        VkCommandBuffer GetCurrentCommandBuffer();
        static VkCommandBuffer CurrentCommandBuffer();
        static VkFramebuffer CurrentFramebuffer();
        static uint32_t GetCurrentCommandBufferIndex();
        
        // static void SubmitSceneObject(Ref<SceneObject>& p_SceneObject);
        // static void SetInitialCamera(Ref<SceneObject>& camera);

    private:
        // void CustomShaderImpl(const std::string& p_VertexShader, const std::string& p_FragmentShader, bool p_IsOverwritten) override;
        void BeginFrame() override;
        void EndFrame() override;
        // void SetInitialCamera(PerspectiveCamera& camera) override;
        void SubmitSceneObject(Ref<SceneObject>& p_SceneObject) override;
        void SetInitialCamera(Ref<SceneObject>& camera) override;
        void DrawScene(Ref<SceneObject> p_SceneContext) override;
        void DrawSceneObjects(const Ref<SceneScope>& p_CurrentScene) override;

        void DrawSceneObject(Ref<SceneObject>& p_CurrentObject) override;

        void DrawObjectWithCamera(Ref<SceneObject>& p_Object, Ref<SceneObject>& p_CameraObject) override;

    private:
        //! @note TODO: Moving VkPipeline/VkPipelineLayout out of the renderer and asbtracting this.
        void InitializeRendererPipeline();
    private:
    };
};