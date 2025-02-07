#include "drivers/ui/imgui_backend.hpp"
#include "scene/scene.hpp"
#include <cstring>
#include <deque>
#include <drivers/vulkan/vulkan_renderer.hpp>
#include <iostream>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <atomic>
#include <drivers/vulkan/vulkan_context.hpp>
#include <drivers/vulkan/helper_functions.hpp>
#include <core/application_instance.hpp>
#include <core/engine_logger.hpp>
#include <drivers/vulkan/vulkan_swapchain.hpp>
#include <drivers/vulkan/shaders/vulkan_shader.hpp>
#include <scene/scene_object.hpp>
#include <numeric>
#include <core/update_handlers/sync_update.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <core/image/stb_image.hpp>

namespace engine3d::vk{
    static std::vector<VkCommandBuffer> g_CommandBuffers;
    VkCommandPool g_CommandPool;
    std::atomic<uint32_t> g_CurrentFrameIndex = -1;
    static bool g_IsFrameStarted = false;
    static VkPipelineLayout g_PipelineLayout;
    // static std::deque<Ref<SceneObject>> g_SceneObjectQueue;
    static std::vector<Ref<SceneObject>> g_SceneObjectQueue;
    // static VkPipeline g_Pipeline;

    static std::deque<std::function<void()>> s_MainDeletionQueue;
    static VkDevice g_Driver = nullptr;

    // uniform buffers and descriptor sets
    // static std::vector<Ref<BufferTutorial>> g_VectorOfUbos;
    // static Ref<BufferTutorial> g_TestUniformBufferObject;
    // static std::vector<Ref<BufferTutorial>> g_UniformBuffers;
    
    // static Ref<vk::DescriptorPool> g_GlobalPool{};
    // static Ref<vk::DescriptorSetLayout> g_GlobalSetLayout{};
    // static std::vector<VkDescriptorSet> g_GlobalDescriptors;

    // Basic Descriptor Set and Uniform buffer usages
    // VkDescriptorSetLayout g_DescriptorSetLayout;

    /**
     * @name UniformBuffer
     * @note TODO: Should probably be its own class that also enables to read/write to it to the shaders
    */
    struct UniformBuffer{
        VkBuffer BufferHanlder;
        VkDeviceMemory BufferMemory;
        void* UniformBufferMappedData;
    };


    // static Ref<SceneObject> g_CurrentSelectedCamera;
    template<typename UFunction>
    void SubmitResourceFree(const UFunction& p_TaskToDelete){
        s_MainDeletionQueue.push_back(p_TaskToDelete);
    }

    //! @note Shader-definitions
    static Ref<Shader> g_Shader = nullptr;

    // THIS IS THE UNIFORM BUFFER TO TEST
    // struct CameraUboTest{
    //     glm::mat4 Projection{1.f};
    //     glm::mat4 View{1.f};
    //     glm::mat4 Model{1.f};
    // };

    struct CameraUbo{
        // glm::mat4 Transform{1.f};
        glm::mat4 Projection{1.f};
        glm::mat4 View{1.f};
        glm::mat4 Model{1.f};
        glm::vec3 LightTransform{1.0, -3.0, -1.0};
        // glm::vec4 Color{0.f};
    };

    //! @note This is just a quick hack to have glm::vec4 color be a push constant
    // struct SceneObjectProperties{
    //     glm::vec4 Color{0.f};
    // };

    struct PointLighTest{
        glm::vec3 Position;
        glm::vec4 Ambient;
        glm::vec3 Color;
    };

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory){
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


        vk_check(vkCreateBuffer(VulkanContext::GetDriver(), &bufferInfo, nullptr, &buffer), "vkCreateBuffer", __FILE__, __LINE__, __FUNCTION__);

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(VulkanContext::GetDriver(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		// allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
        allocInfo.memoryTypeIndex = VulkanContext::GetPhysicalDriver().SearchMemoryType(memRequirements.memoryTypeBits, properties);

		vk_check(vkAllocateMemory(VulkanContext::GetDriver(), &allocInfo, nullptr, &bufferMemory), "vkAllocateMemory", __FILE__, __LINE__, __FUNCTION__);

		vkBindBufferMemory(VulkanContext::GetDriver(), buffer, bufferMemory, 0);
	}
    
    std::vector<UniformBuffer> g_UniformBuffers;
    static void CreateUniformBuffers(){
        g_UniformBuffers.resize(VulkanSwapchain::MaxFramesInFlight);

        for(size_t i = 0; i < VulkanSwapchain::MaxFramesInFlight; i++){
            // Creating our uniform buffers
            createBuffer(sizeof(CameraUbo), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, g_UniformBuffers[i].BufferHanlder, g_UniformBuffers[i].BufferMemory);
        }
    }
    

    static VkDescriptorSetLayout g_DescriptorSetLayout;
    static VkDescriptorSet g_DescriptorSet;


    VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice driver){
        VkDescriptorSetLayoutBinding ubo_layout_binding = {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr
        };

        VkDescriptorSetLayoutCreateInfo desc_set_layout_ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &ubo_layout_binding
        };

        VkDescriptorSetLayout return_set;
        vk::vk_check(vkCreateDescriptorSetLayout(driver, &desc_set_layout_ci, nullptr, &return_set), "vkCreateDescriptorSetLayout", __FILE__, __LINE__, __FUNCTION__);
        return return_set;
    }

    VkDescriptorPool CreatePool(VkDevice driver){
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 1; // Number of descriptor sets you'll allocate

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1; // Maximum number of sets

        VkDescriptorPool descriptorPool;
        vk_check(vkCreateDescriptorPool(driver, &poolInfo, nullptr, &descriptorPool), "vkCreateDescriptorSetLayout", __FILE__, __LINE__, __FUNCTION__);

        return descriptorPool;
    }

    VkDescriptorSet AllocateDescriptorSet(VkDevice driver, VkDescriptorPool Pool, VkDescriptorSetLayout SetLayout){
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = Pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &SetLayout;

        VkDescriptorSet descriptorSet;
        vk_check(vkAllocateDescriptorSets(driver, &allocInfo, &descriptorSet), "vkAllocateDescriptorSets", __FILE__, __LINE__, __FUNCTION__);

        return descriptorSet;
    }


    void VulkanRenderer::InitializeRendererPipeline(){
        g_Driver = VulkanContext::GetDriver();

        // g_UniformBuffers.resize(VulkanSwapchain::MaxFramesInFlight);

        //! @note Creating descriptors
        // CreateDescriptors();
        // CreateUniformBuffers();



        //! @note Setting up push constants
        //! @note VkPipelineLayoutCreateInfo does not require a push constant to be defined in it's configuration
        //! @note It can just be set to its default vulkan has its configuration
        g_CurrentFrameIndex = 0;
        g_IsFrameStarted = false;

        //! @note Setting up our pipeline.
        auto pipeline_config = vk::VulkanShader::shader_configuration(ApplicationInstance::GetWindow().GetWidth(), ApplicationInstance::GetWindow().GetHeight());

        //! @note Initialize Push constant range

        VkPushConstantRange push_const_range = {
            .stageFlags =  VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset = 0,
            .size = sizeof(CameraUbo)
        };

        g_DescriptorSetLayout = CreateDescriptorSetLayout(g_Driver);
        auto set_pool = CreatePool(g_Driver);

        g_DescriptorSet = AllocateDescriptorSet(g_Driver, set_pool, g_DescriptorSetLayout);


        // DescriptorSetInfo info = CreateDescriptors();


        //! @note We are setting our descriptors to work with layout(set = 0, binding = 0)
        // std::vector<VkDescriptorSetLayout> setLayouts;
        // setLayouts.push_back(g_GlobalSetLayout->getDescriptorSetLayout());

        //! @note First initializing pipeline layout create info
        //! @note TODO: Handle if a pipeline layout has descriptors or no
        /*
        
            Pipeline API would look like the following

            Pipeline pipeline_a = Pipeline(descriptors: false, descriptors_count = 0, descriptors: std::span<DescriptorSet>);
            
        */
        VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            // .setLayoutCount = 0,
            // .pSetLayouts = nullptr,
            .setLayoutCount = 1,
            .pSetLayouts = &g_DescriptorSetLayout,
            // .setLayoutCount = 1,
            // .pSetLayouts = &info.Layout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &push_const_range
        };

        vk::vk_check(vkCreatePipelineLayout(vk::VulkanContext::GetDriver(), &pipeline_layout_create_info, nullptr, &g_PipelineLayout), "vkCreatePipelineLayout", __FILE__, __LINE__, __FUNCTION__);
        

        //! @note We are setting our shader pipeline to utilize our current window's swapchain
        //! @note a TODO is to utilize different render passes utiization for shader pipelines, potentially.
        pipeline_config.PipelineRenderPass = ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetRenderPass();
        pipeline_config.PipelineLayout = g_PipelineLayout;

        // m_Shader = Shader::Create("simple_shader/simple_shader.vert.spv", "simple_shader/simple_shader.frag.spv", pipeline_config);
        g_Shader = Shader::Create("shader_ubo_tutorial/simple_shader.vert.spv", "shader_ubo_tutorial/simple_shader.frag.spv", pipeline_config);

        ConsoleLogError("NOT AN ERROR: Shader Loaded Successfully!");


        //! @note Initializing Command buffers.
        g_CommandBuffers.resize(ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetImagesSize());

        VkCommandPoolCreateInfo pool_create_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = vk::VulkanContext::GetPhysicalDriver().GetQueueIndices().Graphics
        };

        vk::vk_check(vkCreateCommandPool(vk::VulkanContext::GetDriver(), &pool_create_info, nullptr, &g_CommandPool), "vkCreateCommandPool", __FILE__, __LINE__, __FUNCTION__);


        ConsoleLogInfo("RENDERER COMMAND BUFFERS SIZE === {}", g_CommandBuffers.size());
        //! @note Allocating our command buffers.
        VkCommandBufferAllocateInfo cmd_buffer_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = g_CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(g_CommandBuffers.size()),
        };

        vk::vk_check(vkAllocateCommandBuffers(vk::VulkanContext::GetDriver(), &cmd_buffer_alloc_info, g_CommandBuffers.data()), "vkAllocateCommandBuffers", __FILE__, __LINE__, __FUNCTION__);

        ConsoleLogInfo("CommandBuffers Size === {}", g_CommandBuffers.size());
    }

    void VulkanRenderer::SetInitialCamera(Ref<SceneObject>& camera){
        // g_CurrentSelectedCamera = camera;
        camera = {};
    }

    VulkanRenderer::VulkanRenderer(const std::string& p_Tag){
        g_CurrentFrameIndex = 0;
        g_IsFrameStarted = false;
        // ConsoleEngineLogger::AddNewLogger(p_Tag);
        if(p_Tag.empty()){}
        g_SceneObjectQueue = std::vector<Ref<SceneObject>>();

        // Initializing Uniform Buffers
        CreateUniformBuffers();

        // initializing descriptors
        // CreateDescriptorSetLayout();

        InitializeRendererPipeline();
    }

    void CleanupPipeline(){
        vkDestroyPipelineLayout(VulkanContext::GetDriver(), g_PipelineLayout, nullptr);
    }

    void VulkanRenderer::BeginFrame(){
        
        if(Swapchain::IsSwapchainRebuild()){
            ConsoleLogFatal("ARE YOU CALED AFTER RESIZE?!?!");
            InitializeRendererPipeline();
            Swapchain::ResetSwapchainRebuildBool();
        }

        //! @note Note to Self: This is just a hack on how to get "resizing" to work
        //! @note I will need to do this better as there are problems with this.
        //! @note Currently the state is if the window is resized by glfwSetFramebufferSizeCallback in vulkan_window.cpp, we also check here for the state of our swapchain.
        //! @note If our swapchain's state is either VK_ERROR_OUT_OF_DATE_KHR or VK_SUBOPTIMAL_KHR, then the Acquired function returns -3.
        //! @note -3 was just to indicate the swapchain's state as been modified and needs to modify the pipeline to work with the new swapchain.
        //! @note Another TODO: Make abstract the pipeline's so that we do not have to modify this directly.
        //! @note If this isn't already done, but the vk::submit function should handle the way we directly handle these tasks... potentially?
        g_CurrentFrameIndex = ApplicationInstance::GetWindow().GetCurrentSwapchain()->AcquireNextImage();
        if(Swapchain::IsSwapchainRebuild()){
            ConsoleLogFatalWithTag("vulkan", "How often are you being called to be initialized?");
            // Renderer::Initialize();
            InitializeRendererPipeline();
        }

        VkCommandBufferBeginInfo cmd_buffer_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };

        auto cmd_buffer = GetCurrentCommandBuffer();

        vk::vk_check(vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin_info), "vkBeginCommandBuffer", __FILE__, __LINE__, __FUNCTION__);
        VkRenderPassBeginInfo rp_begin_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetRenderPass(),
            .framebuffer = ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetFramebuffer(g_CurrentFrameIndex), // Specifying which framebuffer to render pass to.
            .renderArea = {
                .offset = {0, 0},
                .extent = {ApplicationInstance::GetWindow().GetWidth(), ApplicationInstance::GetWindow().GetHeight()}
            }
        };

        std::array<VkClearValue, 2> clearValues;
        clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        rp_begin_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
        rp_begin_info.pClearValues = clearValues.data();


        vkCmdBeginRenderPass(cmd_buffer, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        g_IsFrameStarted = true;
        
        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetSwapchainExtent().width),
            .height = static_cast<float>(ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetSwapchainExtent().height),
            .maxDepth = 1.0f,
        };

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetSwapchainExtent()
        };

        g_CommandBuffers[g_CurrentFrameIndex] = cmd_buffer;

        vkCmdSetViewport(g_CommandBuffers[g_CurrentFrameIndex], 0, 1, &viewport);
        vkCmdSetScissor(g_CommandBuffers[g_CurrentFrameIndex], 0, 1, &scissor);

        ImGuiBackend::Begin();
    }

    void VulkanRenderer::EndFrame(){
        auto current_cmd_buffer = GetCurrentCommandBuffer();

        //! @note Flushing out scene objects
        /*
        for(auto& obj : g_SceneObjectQueue){
        // while(!g_SceneObjectQueue.empty()){
            // SceneObject& object = new_obj;
            SceneObject& object = *obj;

            auto* component = object.GetComponent<MeshComponent>();
            if(component == nullptr){
                ConsoleLogFatal("MeshComponent was invalid that was attempted to get in vulkan_renderer!!!");
                return;
            }
            Mesh mesh_data = component->MeshMetadata;

            auto& vb = mesh_data.GetVertices();
            auto ib = mesh_data.GetIndices();


            vb->Bind(current_cmd_buffer);

            // if(ib != nullptr or ib->HasIndicesPresent()){
            //     ib->Bind(current_cmd_buffer);
            //     ib->Draw(current_cmd_buffer);
            // }
            // else{
            //     vb->Draw(current_cmd_buffer);
            // }

            if(ib != nullptr){
                ib->Bind(current_cmd_buffer);
    
                if(ib->HasIndicesPresent()){
                    ib->Draw(current_cmd_buffer);
                }
                else{
                    vb->Draw(current_cmd_buffer);
                }
            }
            else{
                vb->Draw(current_cmd_buffer);
            }
        }*/
        






        ImGuiBackend::End();
        vkCmdEndRenderPass(current_cmd_buffer);
        vk::vk_check(vkEndCommandBuffer(current_cmd_buffer), "vkEndCommandBuffer", __FILE__, __LINE__, __FUNCTION__);

        ApplicationInstance::GetWindow().GetCurrentSwapchain()->SubmitCommandBuffer(&current_cmd_buffer);
        g_IsFrameStarted = false;
    }


    void VulkanRenderer::DrawScene(Ref<SceneObject> p_SceneContext) {
        if(!p_SceneContext){}
    }

    void VulkanRenderer::DrawSceneObjects(const Ref<SceneScope>& p_Scene){

        if(!p_Scene){}
    }

    void VulkanRenderer::DrawSceneObject(Ref<SceneObject>& p_CurrentObject){
        if(p_CurrentObject == nullptr){}
    }

    void VulkanRenderer::SubmitSceneObject(Ref<SceneObject>& p_Object){
        auto current_cmd_buffer = VulkanRenderer::CurrentCommandBuffer();
        
        vkCmdBindPipeline(current_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_Shader->GetGraphicsPipeline());
        // auto* camera_component = g_CurrentSelectedCamera->GetComponent<engine3d::PerspectiveCamera>();
        PerspectiveCamera* camera_component = nullptr;

        auto obj_model = p_Object->GetModelMatrix();

        float delta_time = sync_update::DeltaTime();

        CameraUbo push_const_data = {
            .Projection = camera_component->GetProjection(),
            .View = camera_component->GetView(),
            .Model = obj_model,
            .LightTransform = (camera_component->Position - p_Object->GetComponent<Transform>()->Position) * delta_time,
        };
        // auto frame_idx = GetCurrentCommandBufferIndex();

        // memcpy(g_UniformBuffers[frame_idx].UniformBufferMappedData, &push_const_data, sizeof(push_const_data));

        vkCmdPushConstants(
            current_cmd_buffer,
            g_PipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(CameraUbo),
            &push_const_data
        );

        g_SceneObjectQueue.push_back(p_Object);
    }
    /**
    TODO: Here are a few issues that I think is whats causing the second mesh to not be rendered

    1.) Because we are binding graphics pipeline. This means that our second mesh is probably getting overridden with the first mesh
        - Still need to look into why thats the case (???)
    
    
    */
    void VulkanRenderer::DrawObjectWithCamera(Ref<SceneObject>& p_Object, Ref<SceneObject>& p_CameraObject){
        // if(p_CameraObject != nullptr){}
        // auto& driver = VulkanContext::GetDriver();

        auto current_cmd_buffer = GetCurrentCommandBuffer();
        
        vkCmdBindPipeline(current_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_Shader->GetGraphicsPipeline());
        // vkCmdBindDescriptorSets(current_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,g_PipelineLayout, 0, 1, &g_DescriptorSet, 0, nullptr);  // Set 0, one descriptor set


        auto* camera_component = p_CameraObject->GetComponent<engine3d::PerspectiveCamera>();
        // auto* camera_component = g_CurrentSelectedCamera->GetComponent<engine3d::PerspectiveCamera>();

        auto obj_model = p_Object->GetModelMatrix();

        CameraUbo push_const_data = {
            .Projection = camera_component->GetProjection(),
            .View = camera_component->GetView(),
            .Model = obj_model,
            .LightTransform = -camera_component->Front
        };
        // push_const_data.LightTransform = {};

        // auto& uniform_buffer = g_UniformBuffers.at(g_CurrentFrameIndex.load());
        auto& uniform_buffer = g_UniformBuffers[g_CurrentFrameIndex.load()];
        if(uniform_buffer.BufferHanlder != nullptr){
        }


        

        VkDescriptorBufferInfo bufferInfoWrite = {};
        bufferInfoWrite.buffer = uniform_buffer.BufferHanlder;
        bufferInfoWrite.offset = 0;
        bufferInfoWrite.range = sizeof(CameraUbo);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = g_DescriptorSet;
        descriptorWrite.dstBinding = 0; // Binding point in the shader
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfoWrite; // Connect to buffer

        vkUpdateDescriptorSets(g_Driver, 1, &descriptorWrite, 0, nullptr);

        // if(g_Driver != nullptr){
        //     ConsoleLogFatal("g_Driver != nullptr!");
        // }

        // void* data;
        // vkMapMemory(g_Driver, uniform_buffer.BufferMemory, 0, sizeof(push_const_data), 0, &data);
        // memcpy(data, &push_const_data, sizeof(push_const_data));
        // vkUnmapMemory(g_Driver, uniform_buffer.BufferMemory);
        // vkMapMemory(g_Driver, uniform_buffer.BufferMemory, 0, sizeof(CameraUbo), 0, &uniform_buffer.UniformBufferMappedData);
        // memcpy(uniform_buffer.UniformBufferMappedData, &push_const_data, sizeof(CameraUbo));
        // vkUnmapMemory(g_Driver, uniform_buffer.BufferMemory);

        vkCmdBindDescriptorSets(current_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,g_PipelineLayout, 0, 1, &g_DescriptorSet, 0, nullptr);  // Set 0, one descriptor set
        // vkCmdBindDescriptorSets(current_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        // g_PipelineLayout, 0, 1, &g_DescriptorSet, 0, nullptr);  // Set 0, one descriptor set



        vkCmdPushConstants(
            current_cmd_buffer,
            g_PipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(CameraUbo),
            &push_const_data
        );
        
        // vkCmdPushConstants(
        //     current_cmd_buffer,
        //     g_PipelineLayout,
        //     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        //     0,
        //     sizeof(SceneObjectProperties),
        //     &properties_push_constant_data
        // );

        g_SceneObjectQueue.push_back(p_Object);
        
        // const MeshComponent* component = p_Object->GetComponent<MeshComponent>();
        SceneObject& object = *p_Object;

        auto* component = object.GetComponent<MeshComponent>();
        if(component == nullptr){
            ConsoleLogFatal("MeshComponent was invalid that was attempted to get in vulkan_renderer!!!");
            return;
        }
        Mesh mesh_data = component->MeshMetadata;

        auto& vb = mesh_data.GetVertices();
        auto ib = mesh_data.GetIndices();


        vb->Bind(current_cmd_buffer);

        if(ib != nullptr || ib->HasIndicesPresent()){
            ib->Bind(current_cmd_buffer);
            ib->Draw(current_cmd_buffer);
        }
        else{
            vb->Draw(current_cmd_buffer);
        }

        // if(ib != nullptr){
        //     ib->Bind(current_cmd_buffer);
 
        //     if(ib->HasIndicesPresent()){
        //         ib->Draw(current_cmd_buffer);
        //     }
        //     else{
        //         vb->Draw(current_cmd_buffer);
        //     }
        // }
        // else{
        //     vb->Draw(current_cmd_buffer);
        // }
        
        
    }

    VkCommandBuffer VulkanRenderer::GetCurrentCommandBuffer(){
        return g_CommandBuffers[g_CurrentFrameIndex];
    }

    VkCommandBuffer VulkanRenderer::CurrentCommandBuffer(){
        return g_CommandBuffers[g_CurrentFrameIndex];
    }

    VkFramebuffer VulkanRenderer::CurrentFramebuffer(){
        return ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetFramebuffer(g_CurrentFrameIndex);
    }

    uint32_t VulkanRenderer::GetCurrentCommandBufferIndex(){
        return g_CurrentFrameIndex;
    }
};