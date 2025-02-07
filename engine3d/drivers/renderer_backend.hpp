#pragma once
#include "scene/scene.hpp"
#include <string>
#include <core/core.hpp>
// #include <scene/scene_object.hpp>

namespace engine3d{
    class SceneObject;
    class RendererContext{
    public:
        static Scope<RendererContext> Initialize(const std::string& p_Tag);
        virtual ~RendererContext() = default;

        void Begin();
        void End();

        static uint32_t GetCurrentFrame();

        // void SetCamera(PerspectiveCamera& camera){ return SetInitialCamera(camera); }
        void SetCameraObject(Ref<SceneObject>& camera) { return SetInitialCamera(camera); }
        void SubmitRenderSceneObject(Ref<SceneObject>& p_Object){ return SubmitSceneObject(p_Object); }

        void RenderSceneObject(Ref<SceneObject>& p_SceneContext);
        void RenderSceneObjects(const Ref<SceneScope>& p_CurrentSceneCtx);
        void RenderWithCamera(Ref<SceneObject>& p_Object, Ref<SceneObject>& p_CameraObject);

        void SetCustomShader(const std::string& p_VertexShader, const std::string& p_FragmentShader, bool p_IsOverwritten);
    private:
        // virtual void CustomShaderImpl(const std::string& p_VertexShader, const std::string& p_FragmentShader, bool p_IsOverwritten) = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        virtual void SubmitSceneObject(Ref<SceneObject>& p_SceneObject) = 0;
        virtual void SetInitialCamera(Ref<SceneObject>& camera) = 0;

        // virtual void SetInitialCamera(PerspectiveCamera& camera) = 0;

        virtual void DrawScene(Ref<SceneObject> p_SceneContext) = 0;
        virtual void DrawSceneObjects(const Ref<SceneScope>& p_CurrentSceneCtx) = 0;
        virtual void DrawSceneObject(Ref<SceneObject>& p_CurrentObject) = 0;
        virtual void DrawObjectWithCamera(Ref<SceneObject>& p_Object, Ref<SceneObject>& p_CameraObject) = 0;
    };
};