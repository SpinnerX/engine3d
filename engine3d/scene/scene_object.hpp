#pragma once
#include <scene/components/components.hpp>
#include <core/engine_logger.hpp>
#include <scene/ecs/entity.hpp>

namespace engine3d{
    /**
     * @name Scene Object
     * @note Defines our object in a given Scene
     * @note Utilizes our wrapper EntityObject that wraps around the flecs API
     * 
     * @note Provides same API as the EntityObject
     * @note While also providing additional API's on how engine3d may define scene objects
    */
    class SceneObject{
    public:
        SceneObject() = default;
        SceneObject(flecs::world* p_Registry, const std::string& p_Tag) : m_Tag(p_Tag), m_Entity(p_Registry, m_Tag){
            m_Entity.AddComponent<Transform>();
            m_Model = glm::mat4(1.0f);
        }

        SceneObject(const flecs::entity& p_Entity) : m_Entity(p_Entity) {}

        ~SceneObject(){
            ConsoleLogFatal("Scene Object Tag = {} HAS DESTRUCTED!!!", m_Tag);
            if(m_Entity.IsAlive()){
                m_Entity.OnDestruction();
            }
        }

        std::string GetTag() const {
            return m_Tag;
        }

        glm::mat4 GetModelMatrix() const { return m_Model; }
        void SetModel(const glm::mat4& p_Other) { m_Model = p_Other; }

        template<typename UComponent>
        void AddComponent(){
            m_Entity.AddComponent<UComponent>();
        }

        template<typename UComponent>
        void AddComponent(UComponent& p_ComponentValue){
            m_Entity.AddComponent<UComponent>(p_ComponentValue);
        }

        template<typename UComponent>
        const UComponent* GetComponent() const {
            return m_Entity.GetComponent<UComponent>();
        }

        template<typename UComponent>
        UComponent* GetMutableComponent(){
            return m_Entity.GetMutableComponent<UComponent>();
        }

        template<typename UComponent>
        bool HasComponent(){
            return m_Entity.HasComponent<UComponent>();
        }

        template<typename UComponent>
        void SetComponent(const UComponent& p_Component){
            m_Entity.SetComponent<UComponent>(p_Component);
        }

        template<typename UComponent, typename UComponent2>
        void SetComponent(const UComponent& p_Component, const UComponent2& p_Component2){
            m_Entity.SetComponent<UComponent>(p_Component, p_Component2);
        }

        template<typename UComponent>
        void RemoveComponent(){
            return m_Entity.RemoveComponent<UComponent>();
        }

        void SetRotation(float p_Angle){
            m_Angle = glm::radians(p_Angle);
        }

        glm::mat4 GetModelMatrix(){
            const Transform* transform_component = GetComponent<Transform>();
            m_Model = glm::mat4(1.f);

            //! @note Anything vec3 has to be vec4 
            m_Model = glm::translate(m_Model, transform_component->Position);
            m_Model = glm::scale(m_Model, transform_component->Scale);
            
            //! TODO: The caveaut of this implicit conversion is, mathematically un-optimized
            //! @note Unoptimized meaning we are squaring quaternions from mat3 to mat4.
            //! @note Squaring a matrix is n^3 operation. slighly faster because of laser method.
            auto rotation_mat4 = glm::mat4(glm::quat(transform_component->Rotation));
            m_Model *= rotation_mat4;

            return m_Model;
        }



    private:
        std::string m_Tag="New Entity";
        EntityObject m_Entity;
        glm::mat4 m_Model;
        float m_Angle = glm::radians(90.0f);
    };
};