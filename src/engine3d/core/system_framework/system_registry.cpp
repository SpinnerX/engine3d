#include <core/system_framework/system_registry.hpp>
#include <core/scene/world.hpp>
#include <core/engine_logger.hpp>

namespace engine3d{
    // static SystemRegistry* s_GlobalSystemRegistry = nullptr;
    // WorldScope* SystemRegistry::s_CurrentWorld;
    // static WorldScope g_CurrentWorld;
    static std::string g_Tag = "Undefined Tag";
    // static WorldScope g_CurrentWorld;
    static std::deque<WorldScope> g_InitialWorlds;
    
    //! @note At initialization what should system registry know?
    //! @note How do we assign a hash as part of the tag?

    void SystemRegistry::InitializeRegistry(){
        ConsoleEngineLogger::AddNewLogger("registry");
    }

    //! @note TODO: Probably want to have g_CurrentWorld in a deque of some sort
    //! @note We need to ensure that a world scope is in synced if there are multiple worlds
    void SystemRegistry::Register(WorldScope* p_World){
        // g_CurrentWorld = *p_World;
        g_InitialWorlds.push_back(*p_World);

        // ConsoleLogInfo("Register::p_World->GetTag() = {}", p_World->GetTag());
        auto current_world = *g_InitialWorlds.begin();
        // ConsoleLogInfo("Register::p_World->GetTag() = {}", p_World->GetTag());
        // ConsoleLogInfo("Register::g_CurrentWorld->GetTag() = {}", current_world->GetTag());

        ConsoleLogInfo("Register::p_World->GetTag() = {}", p_World->GetTag());
        ConsoleLogInfo("Register::g_CurrentWorld->GetTag() = {}", current_world.GetTag());
    }

    /*
        Here is an idea we can use world_scope for

        WorldScope
            - Contains UUID(hash) associated with it
            - All scenes within this world scope contains that has
                Example: SceneScope(Tag: "SceneName#1234")
                         Where name of the scene is "SceneName", world scope hash is "1234" as the initial hash
            - When serializing the UUID gets re-generated every-time
        
        SceneScope
            - Contains its initial tag and the hash of the world it is associated with
            - How the SceneScope gets to know the world its associated is when you register the scene doing, SystemRegistry::Register(this)
            - This can be throw some API like, SystemRegistry::GetWorld(this), where we fetch the scene scope's hash UUID
            - Using this information to getting the world scope with our scene
    */
    WorldScope SystemRegistry::GetWorld(){
        // return g_CurrentWorld;
        return *g_InitialWorlds.begin();
    }
};