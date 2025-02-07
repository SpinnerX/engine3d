#pragma once
#include <core/engine_logger.hpp>
#include <core/update_handlers/timer.hpp>
#include <functional>
#include <deque>
#include <map>
#include <set>

namespace engine3d{
    /**
     * @name Sync Update Manager (SUM)
     * @note Essentially our way of handling differerent state of updates
     * @note Synchronized container to ensure those different updates are synced correctly 
     * 
    */

    // using member_function_t = void(*)();
   
    namespace sync_update{
        using member_function_t = void(*)();
        class subscription_type_info {
        public:

            // subscription_type_info(void* p_Instance, void(*p_CallableObject)()) : instance(p_Instance), m_CallableObject(p_CallableObject) {
            subscription_type_info(void* p_Instance, member_function_t p_CallableObject) : instance(p_Instance), m_CallableObject(p_CallableObject) {
            }

            void operator()(){
                m_CallableObject();
            }

            operator void*(){
                return instance;
            }

        private:
            void* instance;
            member_function_t m_CallableObject;
        };
        /*
        template<typename, typename = std::void_t<>>
        struct HasUpdate : std::false_type{};

        template<typename UCompClass>
        struct HasUpdate<UCompClass, std::void_t<decltype(
            std::declval<UCompClass>().OnUpdate())>> : std::true_type {};

        //! @note lateUpdate specialization
        template<typename, typename = std::void_t<>>
        struct HasLateUpdate : std::false_type{};

        template<typename UCompClass>
        struct HasLateUpdate<UCompClass, std::void_t<decltype(
            std::declval<UCompClass>().LateUpdate())>> : std::true_type {};

        //! @note onTickUpdate specialization
        template<typename, typename = std::void_t<>>
        struct HasPhysicsUpdate : std::false_type{};

        template<typename UCompClass>
        struct HasPhysicsUpdate<UCompClass, std::void_t<decltype(
            std::declval<UCompClass>().PhysicsUpdate())>> : std::true_type {};
        
        template<typename, typename = std::void_t<>>
        struct IsValid : std::false_type {};

        template<typename UObject>
        struct IsValid<UObject, std::void_t<decltype(
            std::declval<UObject>().OnUIUpdate())>> : std::true_type {};
        
        template<typename, typename = std::void_t<>>
        struct IsValid2 : std::false_type {};

        //! @note TODO: REmove this and replace this with batch rendering equivalent
        //! @note batch Rendering is going to do this part but at every frame when EndFrame's called
        template<typename UObject>
        struct IsValid2<UObject, std::void_t<decltype(
            std::declval<UObject>().OnSceneRender())>> : std::true_type {};
        */
        

        //! @note Initializes sync update manager to make sure our manager is able to dispatch our update functions.
        void Initialize();

        //! @note Retrieves the local delta time
        float DeltaTime();

        // Called by threadManager
        void RunUpdate(float deltaTime);

        inline std::deque<std::function<void()>> s_Update;
        inline std::deque<std::function<void()>> s_LateUpdate;
        inline std::deque<std::function<void()>> s_UIUpdate;
        inline std::deque<std::function<void()>> s_RenderQueue;

        // inline std::map<void*, subscription_type_info> s_UpdateLookup;
        // inline std::map<void*, member_function_t> s_Lookup;
        // inline std::set<subscription_type_info> lookup;
        // inline std::deque<subscription_type_info> lookup;

        //! @note We are subscribing our update functions to be dispatched at different times.
        
        //  * @param UObject is the object that contains the defined function benig subscribed to
        //  * @param UFunction is the defined update function that corresponded to the function instance
        
        /*
        
            Functor

            Alternative #1
            using member_function_t = void (*mem_func_t)(void*);

            struct subscription_type_info {
                void* instance; // does not contain any data. only the address to the instance the callable function object is associated with
                void operator()(); // allow for calling this struct as a function using  ()
                // void (*mem_func_t)(void*); // actual callable function object that is associated with it
                member_function_t callable;
            };

            std::deque<std::function<void()>> 
            std::deque<subscription_info> 

            Alternative #2

            struct Subscribable{
                [[no_unique_address]]
            };

            class SceneObject1 : public Subscribable<SceneObject1>{
            public:
                SceneObject1(){
                    subscribe(this, &SceneObject::OnUpdate);
                }
            };

            subcribe(this);

            // Particles



            World Tag
            - Serialization
                - Deserialization
            - Tag #hash_id

            World#hash_id
                - Scene#world_hash_id
            


            // World
            // Tag = "World1#hash_id"
            SystemRegistry::Register(this);

            // Scene

            Option #1
            Register(GetWorld("World1"), this);
            Register(GetWorld(), this);



        */

        /**
         * @note One thing that I'm realizing is that we should only have 4 functions
         * @note These functions will control which phases in the mainloop they are subscribing too
         * @note Reasons for this is because
        */

        // template<typename, typename>
        // struct HasUpdateTest : std::false_type{};

        // template<typename UObject>
        // struct HasUpdateTest<UObject, std::void_t<decltype(std::declval<UObject>().OnUpdate())>> : std::true_type {};
        
        // template<typename UObject>
        // struct HasUpdate<UObject, std::void_t<decltype(
        //     std::declval<UObject>().OnUpdate())>> : std::true_type {};
        
        // template<typename, typename>
        // struct HasUpdate : std::false_type{};

        // template<typename UObject>
        // struct HasUpdate<UObject, std::void_t<decltype(std::declval<UObject>().OnUpdate())>> : std::true_type {};

        // template<typename UObject>
        // struct HasUpdate<UObject, std::void_t<decltype(std::declval<UObject>().OnLateUpdate())>> : std::true_type {};

        /*

            @note I think it is best to subscribe for being multiple various functions
            @note Reason for this is because there is one problem is anybody can pass in a void function despite the different update phases
            @note Yes technically you are checking for those specific function calls but all that is checking are the types rather then the signatures themselves.
            @note I am also realizing that semantically it would make sense to have various functions of 4 for specifically handling those update phases.
            
            @note Question: How do we gurantee that the function we are passing has the signature OnUpdate, OnLateUpdate.
            @note Technically how I plan to have it allows for variations of subscriptions at different update phases.

            @note One thing that we can do is technically have something like either action groups or some structure like map<void*, std::span<function_tr>> that contains addresses to those functions
            @note We are technically having our implementation as various update groups.     
        */


        // template<typename UObject, typename UCallable>
        // inline void told_ya(UObject* p_Instance, const UCallable& p_Callable);

        //! @note Synchronize our update per frame
        template<typename UObject, typename UFunction>
        void sync(UObject* p_Instance, const UFunction& p_Callable){
            static_assert(std::is_member_pointer_v<UFunction>, "Invalid sync function");
            
            s_Update.push_back([p_Instance, p_Callable]() {
                (p_Instance->*p_Callable)();
            });
            // subscription_type_info type_info((void*)p_Instance, (member_function_t&)p_Callable);
            // s_UpdateLookup.insert({(void*)p_Instance, subscription_type_info((void*)p_Instance, (member_function_t&)p_Callable)});
            // s_Lookup.insert({(void*)p_Instance, (member_function_t&)p_Callable});
        }

        //! @note Post update - newly defined late update
        template<typename UObject, typename UCallable>
        inline void post(UObject* p_Instance, const UCallable& p_Callable) {
            static_assert(std::is_member_pointer_v<UCallable>, "Invalid post function");

            s_LateUpdate.push_back([p_Instance, p_Callable]() {
                (p_Instance->*p_Callable)();
            });
        }

        //! @note Submitting our draw calls to be rendered
        //! TODO: Should move this away from the user
        template<typename UObject, typename UCallable>
        inline void submit(UObject* p_Instance, const UCallable& p_Callable) {
            static_assert(std::is_member_pointer_v<UCallable>, "Invalid submit function");
            s_RenderQueue.push_back([p_Instance, p_Callable]() {
                (p_Instance->*p_Callable)();
            });
        }

        //! @note Attach our UI Updates
        template<typename UObject, typename UCallable>
        void attach(UObject* p_Instance, const UCallable& p_Callable){
            // UI returns a boolean because this can be used to indicate whether specific callable functions are activated
            //! @note NOTE: This is just for testing...
            static_assert(std::is_member_pointer_v<UCallable>, "Invalid attach function");
            
            s_UIUpdate.push_back([p_Instance, p_Callable](){
                (p_Instance->*p_Callable)();
            });
        }


        
        


        /*
        template<typename UObject, typename UFunction>
        inline void Subscribe(UObject* p_Instance, const UFunction& p_Update) {
            if constexpr (IsUpdateValid<UObject>){
                ConsoleLogTrace("IsUpdateValid === true!");
                if constexpr (requires(decltype(p_Instance)) { p_Instance->OnUpdate(); }){
                    ConsoleLogTrace("OnUpdate Valid!");
                    s_SyncUpdateSubscribers.push_back([p_Instance, p_Update]() {
                        (p_Instance->*p_Update)();
                    });
                }
                else if constexpr (requires(decltype(p_Instance)) { p_Instance->OnLateUpdate(); }){
                    ConsoleLogTrace("OnUpdate Valid!");
                }
                else if constexpr (requires(decltype(p_Instance)) { p_Instance->OnLateUpdate(); }){
                    ConsoleLogTrace("OnUpdate Valid!");
                }
            }
            else{
                static_assert(IsUpdateValid<UObject>, "Invalid!");
            }

            
            if constexpr (HasUpdate<UObject>::value){
                ConsoleLogFatal("OnSceneRender should be subscribing! up");
                if(&UObject::OnUpdate == p_Update){
                    s_SyncUpdateSubscribers.push_back([p_Instance, p_Update]() {
                        (p_Instance->*p_Update)();
                    });
                }
            }
            else if constexpr (HasLateUpdate<UObject>::value){
                 ConsoleLogFatal("OnSceneRender should be subscribing! lup");
                if(&UObject::LateUpdate == p_Update){
                    s_SyncLateUpdateSubscribers.push_back([p_Instance, p_Update]() {
                        (p_Instance->*p_Update)();
                    });
                }
            }
            else if constexpr (HasPhysicsUpdate<UObject>::value){
                 ConsoleLogFatal("OnSceneRender should be subscribing! ph");
                if(&UObject::PhysicsUpdate == p_Update)
                {
                    s_SyncOnTickUpdateSubscribers.push_back([p_Instance, p_Update]() {
                        (p_Instance->*p_Update)();
                    });
                }
            }
            else if constexpr (IsValid2<UObject>::value){
                 ConsoleLogFatal("OnSceneRender Subscribed!");
                s_SyncRenderUpdate.push_back([p_Instance, p_Update]() {
                    (p_Instance->*p_Update)();
                });;
            }
            else{
                // ConsoleLogFatal("Faulted Subscribing Function!");
                // throw std::runtime_error("Hello");
                static_assert((HasUpdate<UObject>::value || HasLateUpdate<UObject>::value || HasLateUpdate<UObject>::value || IsValid2<UObject>::value), "Invalid callable parameter!");
            }
            

            // static_assert(IsValid<UObject>::value, "invalid callable function passed into connect widget");
        }

        // using member_function_t = void(*m_func_t)(void*);
        // template<typename UObject, typename UFunction>
        // // static void Subscribe2(void* p_Instance,  void(*p_CallableObject)(void*)){
        // static void Subscribe2(UObject* p_Instance, const UFunction& p_CallableObject){
        //     // s_UpdateTypeInformation.push_back(subscription_type_info(p_Instance, p_CallableObject));
        //     subscription_type_info type_info((void*)p_Instance, (subscription_type_info::member_function_t&)p_CallableObject);
        //     s_UpdateTypeInformation.push_back(type_info);
        //     s_UpdateTypeInformationLookup.insert({type_info, type_info});
        // }

        
        template<typename UObject, typename UCallableFunction>
        void ConnectWidget(UObject* p_Instance, const UCallableFunction&& p_Callable){
            if constexpr (IsValid<UObject>::value){
                if(&UObject::OnUIUpdate == p_Callable){
                    // UI returns a boolean because this can be used to indicate whether specific callable functions are activated
                    //! @note NOTE: This is just for testing...
                    s_SyncUIUpdate.push_back([p_Instance, p_Callable](){
                        (p_Instance->*p_Callable)();
                    });
                }
            }
            // else{
            //     ConsoleLogFatal("Subscibe Function was not ConnectWidget!");
            //     return;
            // }
            // static_assert(IsValid<UObject>::value, "invalid callable function passed into connect widget");
        }

        template<typename UObject, typename UCallableFunction>
        void SubscribeOnRender(UObject* p_Instance, const UCallableFunction& p_Callable){
            if constexpr (IsValid2<UObject>::value){
                ConsoleLogFatal("OnSceneRender should be subscribing! re");
                if(&UObject::OnSceneRender == p_Callable)
                {
                    ConsoleLogFatal("OnSceneRender Subscribed!");
                    s_SyncRenderUpdate.push_back([p_Instance, p_Callable]() 
                    {
                        (p_Instance->*p_Callable)();
                    });
                    return;
                }
            }
        }
        */


        void OnUpdate();

        // static void OnUpdateSub2();

        void OnUIUpdate();

        void OnSceneRender();

        // Called EveryFrame
        // void OnPhysicsUpdate();
    // private:

        // // Varied by random frames
        // static void OnUpdate();
        void OnLateUpdate();
        // static void OnSceneRender();

        // This IsRender template specialization is not needed.
        //! @note update specialization
        // template<typename, typename = std::void_t<>>
        // struct IsRenderer : std::false_type{};

        // // RenderScenes (?)
        // // This, I will comment out.
        // // // I am considering replacing this with the scene renderer. When I start looking into the scene graph stuff.
        // template<typename UCompClass>
        // struct IsRenderer<UCompClass, std::void_t<decltype(
        //     std::declval<UCompClass>().RenderScenes())>> : std::true_type {};

        //! @note update specialization
        /*
        template<typename, typename = std::void_t<>>
        struct HasUpdate : std::false_type{};

        template<typename UCompClass>
        struct HasUpdate<UCompClass, std::void_t<decltype(
            std::declval<UCompClass>().OnUpdate())>> : std::true_type {};

        //! @note lateUpdate specialization
        template<typename, typename = std::void_t<>>
        struct HasLateUpdate : std::false_type{};

        template<typename UCompClass>
        struct HasLateUpdate<UCompClass, std::void_t<decltype(
            std::declval<UCompClass>().LateUpdate())>> : std::true_type {};

        //! @note onTickUpdate specialization
        template<typename, typename = std::void_t<>>
        struct HasPhysicsUpdate : std::false_type{};

        template<typename UCompClass>
        struct HasPhysicsUpdate<UCompClass, std::void_t<decltype(
            std::declval<UCompClass>().PhysicsUpdate())>> : std::true_type {};

        
        template<typename, typename = std::void_t<>>
        struct IsValid : std::false_type {};

        template<typename UObject>
        struct IsValid<UObject, std::void_t<decltype(
            std::declval<UObject>().OnUIUpdate())>> : std::true_type {};


        template<typename, typename = std::void_t<>>
        struct IsValid2 : std::false_type {};

        //! @note TODO: REmove this and replace this with batch rendering equivalent
        //! @note batch Rendering is going to do this part but at every frame when EndFrame's called
        template<typename UObject>
        struct IsValid2<UObject, std::void_t<decltype(
            std::declval<UObject>().OnSceneRender())>> : std::true_type {};
        */

    // private:
        //! @note is there a better way of doing this?
        // static std::deque<std::function<void()>> s_SyncLateUpdateSubscribers;
        // static std::deque<std::function<void()>> s_SyncUpdateSubscribers;
        // static std::deque<std::function<void()>> s_SyncOnTickUpdateSubscribers;
        // static std::deque<std::function<void()>> s_SyncUIUpdate;
        // static std::deque<std::function<void()>> s_SyncRenderUpdate;

        // static std::deque<subscription_type_info> s_UpdateTypeInformation;

        //! @note The idea is we utilize the map as the lookup
        /*
        
            void* - to only contain the memory address to the location of where the callable function object's located
            void(*m_func_t)(); - contains the actual callable function object to use
            operator()() - to indicate this call as a function call without directly modifying the data in itself.


            Map
                - This map takes in the key
                    - Key represents the memory location of the object's callable object
                    - We know where this object is stored. Therefore what we could do is tell where this location is to do the unsubscripe operation

                - Whereas the value of the map is:
                    - containing the type information of the function callable object and the location where this function is coming from
                
                void* wont contain all of the data of the object we are associating it with. Rather it just takes in the memory address location ONLY to that object
        
        
        */
        // static std::map<void*, subscription_type_info> s_UpdateTypeInformationLookup;
    };

    //! @note These are public functions that users can utilize
    //! @note If users are using anything within the sync_update namespace. They shouldn't!
    template<typename UObject, typename UCallable>
    void sync(UObject* p_Instance, const UCallable& p_Callable){
        sync_update::sync(p_Instance, p_Callable);
    }

    template<typename UObject, typename UCallable>
    void post(UObject* p_Instance, const UCallable& p_Callable){
        sync_update::post(p_Instance, p_Callable);
    }

    template<typename UObject, typename UCallable>
    void submit(UObject* p_Instance, const UCallable& p_Callable){
        sync_update::submit(p_Instance, p_Callable);
    }


    template<typename UObject, typename UCallable>
    void attach(UObject* p_Instance, const UCallable& p_Callable){
        sync_update::attach(p_Instance, p_Callable);
    }



};