#pragma once
#include <core/scene/scene.hpp>
#include <map>

namespace engine3d{
    /**
     * @name LevelScene
     * @note Implementation of our scene
     * @note Meaning this is just the first iteration of how we can look at these scenes and how they can be defined
     * @note Each scene is a type of scene scope, this means that when a player leaves the outer scope of a scene,
     *       the player transitions into a different scene
     * @note Either the player transitions into a different scene or hits a invisible wall preventing leaving the scope
    */
    class LevelScene : public SceneScope{
    public:
        LevelScene() = default;

        LevelScene(const std::string& p_Tag);

        virtual ~LevelScene(){}

        //! @note Remove these from being overridden
        //! TODO: We should have an indication on what functions are update phased functions
        void OnUpdate();

        void OnUIUpdate();

        //! TODO: Replacing this with batch rendering
        //! @note Typically EndFrame would handle this
        //! TODDO: Remove this function. Remove the subscribe for the function.
        //! TODO: Provide ways of submitting scene objects from a scene to the rendderer. Rather having the scene renderer deal with that
        void OnSceneRender();

        void OnLateUpdate(){}

        void TestingFunction(){}



    private:
        Ref<SceneObject> m_Sphere;
        Ref<SceneObject> m_Platform;
        Ref<SceneObject> m_Camera;
        std::vector<Ref<SceneObject>> m_MoreObjects;
        std::map<uint32_t, Ref<SceneObject>> m_SceneObjectLookup;
    };
};