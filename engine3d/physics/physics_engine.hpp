#pragma once
#include <string>
#include <physics/jolt-cpp/jolt-imports.hpp>

namespace engine3d::physics{
    /**
     * @note Here are a few things to do
     * TODO: Handle various entry points to a scene. Such as PhysicsScene (which essentially just wraps our scene)
     * TODO: Looking at how to handle different scene physics. For ensuring that when we transition to a different scene the physics isnt using data from previous scene
     * TODO: timer namespace, specifically for having different timers
     *      - Timer for physics
     *      - Timer for rendering (may utilize the same timer that is used throughout the engine)
     * @note This is because physics timers need have to be faster then the timer the rendering uses
     * 
     * 
     * 
     * @name Physics Engine
     * @note Engine3D's abstraction layer for specifically porting over jolt's or potentially any other physics framework into engine3d
     * 
     * 
     * Here are where I believe a physics system should reside
     * 
     * Scenes
     *  - Having some class thats called PhysicScene. Physics Scene is responsible for the scene's physics
     *  - Each of these "PhysicScene" contains something called physics::settings
     *  - physics::setting contains properties such as gravity and other environmental parameters that can be modified for that specific targeted scene
    */

    //! @note Used for indicating whether objects in a given scene are static or dynamic (meaning can collide with other objects)
    //! @note Used as a representation for both ObjectLayer(uint16_t) and BroadPhaseLayer(class)

    /**
     * @note ObjectLayer just seems to be defined as a body type within joltphysics
     * @note BroadPhaseLayer is just joltphysic's representation of a collider pretty much. It is mainly used by the jolt physics collision
     *       detection system that uses it. Typically in JoltPhysics you'd have less of these then ObjectLayer's it seems
     * 
     * @note BroadPhaseLayer can be ways to map multiple object layers (BodyType's) 
    */
    enum class BodyType : uint8_t{
        UNDEFINED=0, STATIC=1, DYNAMIC=2, KINEMATIC=3
    };

    namespace phase_layer{
        static constexpr JPH::BroadPhaseLayer STATIC(0);
    };

    //! @note Physics settings per physic scene
    struct setting{
        float gravity = -10.f;
    };

    class PhysicBody{
    public:
        //! @note Physics bodies are typically static
        PhysicBody(uint32_t p_BodyID = -1, BodyType p_Type = BodyType::STATIC);

    private:
    };

    class PhysicSystem{
    public:
        PhysicSystem(const setting& p_Settings = {});
    };
};  