#include "drivers/ui/imgui_backend.hpp"
#include "renderer/renderer.hpp"
#include "update_handlers/sync_update.hpp"
#include <core/application_instance.hpp>
#include <core/engine_logger.hpp>
#include <core/event/input_poll.hpp>
#include <core/update_handlers/global_update.hpp>
#include <imgui.h>
#include <string>

namespace engine3d{
    static std::string g_Tag = "engine3d";
    ApplicationInstance* ApplicationInstance::s_ThisInstance = nullptr;
    static API g_GraphicBackendAPI = API::UNSPECIFIED;

    ApplicationInstance::ApplicationInstance(const std::string& p_Tag){
        s_ThisInstance = this;
        g_Tag = p_Tag;
        ConsoleEngineLogger::SetCurrentApplicationTagLogger(g_Tag);
        SetCurrentAPI(API::VULKAN);
        m_Window = Window::Create(1200, 800, g_Tag);
        
        Renderer::Initialize();
        ImGuiBackend::Initialize();
    }

    void ApplicationInstance::SetCurrentAPI(API api){
        g_GraphicBackendAPI = api;
    }

    Ref<Swapchain> ApplicationInstance::GetCurrentSwapchani(){
        return GetWindow().GetCurrentSwapchain();
    }

    API ApplicationInstance::CurrentAPI(){
        return g_GraphicBackendAPI;
    }

    void ApplicationInstance::ShutdownApplication(){
        Super()->GetWindow().Close();
    }

    void ApplicationInstance::ExecuteMainloop(){
        ConsoleLogInfo("Executing mainloop!");

        while(m_Window->IsWindowActive()){
            InputPoll::Update();
            
            Renderer::Begin();
            GlobalUpdate::GlobalOnTickUpdate();

            //! TODO: Submit ImGuiBackend::Begin so we can have the UI also be on the renderer thread
            //! @note So we can make sure when submitting UI-stuff are in synced with the renderer in terms of fetching images
            //! TODO: ImGuiBackend::Begin/End() is needed be called after everything gets rendered.
            //! TODO: UI gets rendered last
            // ImGuiBackend::Begin();
            // if(ImGui::Begin("Setting")){
            //     ImGui::Button("Pres Me!");
            //     ImGui::End();
            // }
            // ImGuiBackend::End();

            // GlobalUpdate::UpdateUI();

            // SyncUpdate::OnUIUpdate();

            Renderer::End();
        }
        ConsoleLogWarn("Leaving executed mainloop!");
    }

    uint32_t ApplicationInstance::GetAspectRatio(){
        return GetWindow().GetWidth() / GetWindow().GetHeight();
    }
};