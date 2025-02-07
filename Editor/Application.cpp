#include <core/update_handlers/sync_update.hpp>
#include <core/engine_logger.hpp>
#include <core/update_handlers/global_update.hpp>
#include <core/application_instance.hpp>
#include "EditorWorld.hpp"
#include <exception>
#include <renderer/renderer.hpp>

namespace engine3d{
    /*
        For Reference Here are what Application should handle

        Project Settings
            - Preconfigurations that are set by the user in the editor that are set via some config file
        
        Application Settings
            - Some sort of settings to modify graphics usage, and UI HUD to do something like that
        
        Serialization/Deserialization
            - Deserializes into pre-init state to the actual game state
        
        What Application shouldnt do
            - Should not handle updates, syncing updates, subscribing updates
            - Should not handle creating instances inside but dispatching to our system registry to do manage object lifetimes
    
    */
    void handler(){
        // printf("Caught!!!\n");
        // std::current_exception();
    }
    class Application : public ApplicationInstance{
    public:
        Application(const std::string& p_Tag) : ApplicationInstance(p_Tag){
            ConsoleLogFatal("Application::Application(std::string) gets called!");
            m_World = CreateRef<EditorWorld>("Editor World");

            // GlobalUpdate::SubscribeApplicationUpdate(this, &Application::OnApplicationUpdate);
            // std::set_terminate(handler);
            // throw std::runtime_error("Its meee!!!");
        }

        // void OnApplicationUpdate(){
        //     //! TODO: Currently renderer is being called per-level. This is not what we want.
        //     //! @note What needs to happen is we provide a scene renderer.
        //     /*
        //     Scene Renderer
        //     - Handles multiple-passes for us.
        //     - Uses the draw call API's from the renderer to interface with the vulkan's API to do specific operations
        //     - Whether this is drawing 2D line, 3D objects, telling which uniforms to update, etc
        //     */
        //     // m_World->OnUpdate();
        // }

    private:
        Ref<EditorWorld> m_World;
    };

    Ref<ApplicationInstance> Initialize(){
        return CreateRef<Application>("Editor");
    }
};