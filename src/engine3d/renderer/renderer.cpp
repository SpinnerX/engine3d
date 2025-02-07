#include <renderer/renderer.hpp>
#include <drivers/renderer_backend.hpp>

namespace engine3d{
    //! @note Only this renderer should have ownership of this renderer context
    //! @note Renderer context is  a graphics-agnostic renderer that provides an API to interact with our agnostic graphcis API's such as Vulkan, DirectX (if we plan to), etc.
    Scope<RendererContext> g_RendererBackend = nullptr;

    void Renderer::Initialize(){
        g_RendererBackend = RendererContext::Initialize("Renderer");
    }

    // void Renderer::SetCustomShaders(const std::string& p_VertexShader, const std::string& p_FragmentShader, bool p_IsOverwritten){
    // }

    void Renderer::Begin(){
        return g_RendererBackend->Begin();
    }

    void Renderer::End(){
        return g_RendererBackend->End();
    }

    uint32_t Renderer::GetCurrentFrame(){
        return RendererContext::GetCurrentFrame();
    }

    // void Renderer::SetInitialCamera(PerspectiveCamera& camera){
    //     g_RendererBackend->SetCamera(camera);
    // }

    void Renderer::SetCamera(Ref<SceneObject>& object){
        g_RendererBackend->SetCameraObject(object);
    }

    void Renderer::SubmitSceneObject(Ref<SceneObject>& p_Object){
        g_RendererBackend->SubmitRenderSceneObject(p_Object);
    }

    void Renderer::RenderSceneObjects(const Ref<SceneScope>& p_CurrentSceneContext){
        return g_RendererBackend->RenderSceneObjects(p_CurrentSceneContext);
    }

    void Renderer::RenderSceneObject(Ref<SceneObject>& p_ObjectToRender){
        g_RendererBackend->RenderSceneObject(p_ObjectToRender);
    }

    void Renderer::RenderWithCamera(Ref<SceneObject>& p_Object, Ref<SceneObject>& p_CameraObject){
        g_RendererBackend->RenderWithCamera(p_Object, p_CameraObject);
    }
};