#include "LevelScene.hpp"
#include "core/application_instance.hpp"
#include "drivers/vulkan/vulkan_context.hpp"
#include <core/update_handlers/sync_update.hpp>
#include <core/event/input_poll.hpp>
#include <imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <renderer/renderer.hpp>
#include <core/ui/widgets.hpp>
#include <vulkan/vulkan_core.h>
#include <drivers/vulkan/helper_functions.hpp>
#include <core/image/stb_image.hpp>

#include <physics/jolt-cpp/jolt-imports.hpp>

namespace engine3d{
    struct MeshData{
        glm::vec3 Position{0.f};
        glm::vec3 Scale{0.f};
        glm::vec3 Rotation{0.f};
        std::string mesh_file = "";
        glm::vec4 Color{0.f};
    };

    struct CameraData{
        glm::vec3 Position{0.f};
        glm::vec3 Front{0.f};
        float Angle = 90.f;
    };

    struct TargetCamera{
        glm::vec3 Position{0.f};
        glm::vec3 Front{0.f};
        glm::vec3 Up{0.f};
        glm::vec3 Target{0.f};
    };

    static VkSampler g_TextureSampler;

    struct VulkanTextureData{
        VkImage Image;
        VkImageView ImageView;
        VkSampler Sampler;
    };

    // static VulkanTextureData g_Texture;

    static std::vector<VkImageView> g_ViewportImages;

#if 0
    static VkImage CreateImage(uint32_t Width, uint32_t Height, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties, VkImage& Image, VkDeviceMemory& ImageMemory){
        VkImageCreateInfo image_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = Format,
            .extent = {
                .width = Width,
                .height = Height,
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = Tiling,
            .usage = Usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VkImage image;

        vk::vk_check(vkCreateImage(vk::VulkanContext::GetDriver(), &image_ci, nullptr, &image), "vkCreateImage", __FILE__, __LINE__, __FUNCTION__);
        
        return image;
    }

    static VkImageView CreateImageView(VkImage Image, VkFormat format, VkImageAspectFlags aspect_flags){
        VkImageView image_view;
        VkImageViewCreateInfo image_view_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .image = Image,
            .format = format,
            .subresourceRange = {
                .aspectMask = aspect_flags,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        vk::vk_check(vkCreateImageView(vk::VulkanContext::GetDriver(), &image_view_ci, nullptr, &image_view), "vkCreateImageView", __FILE__, __LINE__, __FUNCTION__);

        return image_view;
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(vk::VulkanContext::GetDriver(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(vk::VulkanContext::GetDriver(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = vk::VulkanContext::GetPhysicalDriver().SearchMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(vk::VulkanContext::GetDriver(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(vk::VulkanContext::GetDriver(), buffer, bufferMemory, 0);
	}

    static void CreateTextureImage(VulkanTextureData TextureImage){
        int texWidth, texHeight, texChannels;
		stbi_uc *pixels = stbi_load(("/textures/texture.jpeg").c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels)
		{
			throw std::runtime_error("failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void *data;
		vkMapMemory(vk::VulkanContext::GetDriver(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(vk::VulkanContext::GetDriver(), stagingBufferMemory);

		stbi_image_free(pixels);

		createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory);

		transitionImageLayout(TextureImage.Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(stagingBuffer, TextureImage.Image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

		transitionImageLayout(TextureImage.Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(vk::VulkanContext::GetDriver(), stagingBuffer, nullptr);
		vkFreeMemory(vk::VulkanContext::GetDriver(), stagingBufferMemory, nullptr);
    }

    static void InitializeViewportImageViews(){
        g_ViewportImages.resize(ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetImagesSize());

        for(size_t i = 0; i < g_ViewportImages.size(); i++){
            // g_ViewportImages[i] = CreateImageView(VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT)
        }
    }
#endif

    static VkSampler CreateSampler(){
        VkSampler return_sampler;
        VkSamplerCreateInfo sampler_ci = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.0f,
            .anisotropyEnable = false,
            .maxAnisotropy = 1.0f,
            .compareEnable = false,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
            .unnormalizedCoordinates = false,
        };

        vk::vk_check(vkCreateSampler(vk::VulkanContext::GetDriver(), &sampler_ci, nullptr, &return_sampler), "vkCreateSampler", __FILE__, __LINE__, __FUNCTION__);
        return return_sampler;
    }


    // struct TargetCamera{
    //     // Transform transform;
    //     Transform target; // target of the camera that it is following
    //     glm::vec3 LookDown;
    //     uint32_t Offset = -11; // offset of the camera's z-axis
    //     uint32_t CameraHeight = 7;
    // };

    /*
    JPH::ObjectLayer = uint16_t
    static constexpr JPH::ObjectLayer Static = 0;
	static constexpr JPH::ObjectLayer Kenmatic = 1;
    static constexpr JPH::ObjectLayer Dynamic = 2;
	static constexpr JPH::ObjectLayer NUM_LAYERS = 3;
    */

    // static constexpr JPH::BroadPhaseLayer Static(0);
	// static constexpr JPH::BroadPhaseLayer Kenmatic(1);
    // static constexpr JPH::BroadPhaseLayer Dynamic(2);
	// static constexpr JPH::uint NUM_LAYERS(3);

    static std::vector<VkDescriptorSet> g_DescriptorSets;

    static float sensitivity = 0.f;

    static MeshData sphere_data;
    static MeshData some_mesh_data;
    static CameraData camera_data;
    static std::string g_MeshFilepath = "";

    struct Projectile{
        glm::vec3 Position;
    };

    void OnAttach(){
        printf("ONAttach should not work!!!\n");
    }


    LevelScene::LevelScene(const std::string& p_Tag) : SceneScope(p_Tag){
        ConsoleLogInfo("SceneScope::SceneScope with Tag = {} called!", p_Tag);

        WorldScope world = SystemRegistry::GetWorld();
        std::string world_tag = world.GetTag();
        ConsoleLogTrace("From LevelScene::LevelScene() ==> World Tag = {}", world_tag);

        // Renderer::OverrideShader("filepath/custom.vs", "filepath/custom.fs", true);

        //! @note Creating our objects from our scene
        m_Sphere = this->CreateNewObject("sphere");
        m_Sphere->SetComponent<MeshComponent>({"3d_models/tutorial/colored_cube.obj"});
        m_Sphere->AddComponent<Projectile>();
        // m_Sphere->SetComponent<MeshComponent>({"3d_models/tutorial/quad.obj"});
        // m_Sphere->SetComponent<MeshComponent>({"3d_models/tutorial/sphere.obj"});
        m_Sphere->SetComponent<Transform>({
            .Position = {0.f, 2.10f, -7.30f},
            .Scale = {.20f,.20f, .20f},
            .Color = {1.0f, 0.f, 0.f, 0.f}
        });

        auto transform = m_Sphere->GetComponent<Transform>();
        
        sphere_data.Position = transform->Position;
        sphere_data.Scale = transform->Scale;
        sphere_data.Rotation = transform->Rotation;

        m_Platform = this->CreateNewObject("mesh1");
        m_Platform->SetComponent<Transform>({
            .Position = {0.f, 1.40f, -7.4f},
            .Scale = {2.80f, -0.08f, 3.50f}
        });

        auto some_mesh_transform = m_Platform->GetComponent<Transform>();
        some_mesh_data.Position = some_mesh_transform->Position;
        some_mesh_data.Scale = some_mesh_transform->Scale;
        some_mesh_data.Rotation = some_mesh_transform->Rotation;
        m_Platform->SetComponent<MeshComponent>({"3d_models/tutorial/cube.obj"});

        m_Camera = this->CreateNewObject("camera");

        camera_data.Position = {0.0f, 1.50f, 0.0f};
        camera_data.Front = glm::vec3(-0.0f, 0.0f, -1.0f);

        m_Camera->AddComponent<PerspectiveCamera>();

        sensitivity = m_Camera->GetComponent<PerspectiveCamera>()->MovementSpeed;
        g_TextureSampler = CreateSampler();

        g_DescriptorSets.resize(ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetImagesSize());
        ConsoleLogTrace("g_DescriptorSets.size() = {}", g_DescriptorSets.size());
        for(size_t i = 0; i < g_DescriptorSets.size(); i++){
            g_DescriptorSets[i] = ImGui_ImplVulkan_AddTexture(g_TextureSampler, ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetImageView(i), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        // m_MoreObjects.resize(1000);

        
        // MeshComponent mesh = {"3d_models/tutorial/sphere.obj"};
        
        // int counter = 0;
        // for(size_t x= 0; x < 10; x++){
        //     for(size_t y= 0; y < 10; y++){
        //         for(size_t z = 0; z < 10; z++){
        //             m_MoreObjects[counter] = this->CreateNewObject(fmt::format("Object {}", counter));
        //             m_MoreObjects[counter]->AddComponent<MeshComponent>(mesh);
        //             m_MoreObjects[counter]->SetComponent<Transform>({
        //                 .Position = {x,y,z},
        //                 .Scale = {.20f,.20f, .20f}
        //             });

        //             m_SceneObjectLookup.insert({counter, m_MoreObjects[counter]});
        //             counter++;
        //         }
        //     }
        // }

        // SyncUpdate::Subscribe2(this, &LevelScene::OnUpdate);
        // sync_update_manager::subscribe(this, &LevelScene::OnUpdate);
        sync(this, &LevelScene::OnUpdate);
        submit(this, &LevelScene::OnSceneRender);
        attach(this, &LevelScene::OnUIUpdate);
    }

    void LevelScene::OnUIUpdate(){
        // ConsoleLogTrace("OnUIUpdate called!");
        // DockspaceWindow(ApplicationInstance::GetWindow());

        if(ImGui::Begin("Viewport")){
            // ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            glm::vec2 viewportPanelSize = glm::vec2{ApplicationInstance::GetWindow().GetWidth(), ApplicationInstance::GetWindow().GetHeight()};

            auto frame_idx = Renderer::GetCurrentFrame();
            // ConsoleLogTrace("Current Frame = {}", frame_idx);
            // auto frame_idx = ApplicationInstance::GetWindow().GetCurrentSwapchain()->AcquireNextImage();
            // auto frame_view = ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetImageView(frame_idx);
            if(ImGui::Begin("Image")){
                ImGui::Image(g_DescriptorSets[frame_idx], ImVec2{1, 1}, ImVec2{0, 1}, ImVec2{1, 0});
                ImGui::End();
            }
            ImGui::End();
        }

        if(ImGui::Begin("Settings")){
            // static bool dockspace_open = true;
            // ImGui::ShowDemoWindow(&dockspace_open);
            ImGui::End();
        }


        if(ImGui::Begin("Scene Heirarchy")){
            // ImGui::Image(0, ImVec2(ApplicationInstance::GetWindow().GetWidth(), ApplicationInstance::GetWindow().GetHeight()), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::End();
        }

        if(ImGui::Begin("Content Browser")){
            ImGui::End();
        }


        if(ImGui::Begin("Properties Panel")){

            //! @note THERE IS AN ERROR. Where if the imgui docking window is outside of the window
            //! @note Imgui will just have a window that appears until when you exit the application and the UI is not docked outside the window
            ui::DrawPanelComponent<MeshComponent>("Sphere", [&](){
                ui::DrawVec3UI("pos 1", sphere_data.Position);
                ui::DrawVec3UI("scale 1", sphere_data.Scale);
                ui::DrawVec3UI("rotate 1", sphere_data.Rotation);
                ui::LoadFileWithUI("Load Mesh 1", sphere_data.mesh_file);
                // ConsoleLogTrace("mesh_file = {}", sphere_data.mesh_file);
                
                if(sphere_data.mesh_file != ""){
                    std::filesystem::path relative_path = std::filesystem::relative(sphere_data.mesh_file, "./");
                    ConsoleLogTrace("Filepath = {}", sphere_data.mesh_file);
                    ConsoleLogTrace("in branch 1 mesh_file = {}", relative_path.string());
                    // m_Sphere->SetComponent<MeshComponent>({"3d_models\\tutorial\\Ball OBJ.obj"});
                    m_Sphere->SetComponent<MeshComponent>({relative_path.string()});
                    //! TODO: Empty String again 
                    sphere_data.mesh_file = "";
                }
            });

            ui::DrawPanelComponent<MeshComponent>("Some Mesh", [&](){
                ui::DrawVec3UI("Position 2", some_mesh_data.Position);
                ui::DrawVec3UI("Scale 2", some_mesh_data.Scale);
                ui::DrawVec3UI("Rotation 2", some_mesh_data.Rotation);
                ui::DrawFloatUI("Mouse Sensitivity", sensitivity);
                ui::LoadFileWithUI("Load Mesh 2", some_mesh_data.mesh_file);
                // ImGui::Button("")

                // ConsoleLogTrace("mesh_file = {}", some_mesh_data.mesh_file);

                if(some_mesh_data.mesh_file != ""){
                    std::filesystem::path relative_path = std::filesystem::relative(some_mesh_data.mesh_file, "./");
                    ConsoleLogTrace("in branch 2 mesh_file = {}", relative_path.filename().string());
                    m_Platform->SetComponent<MeshComponent>(MeshComponent{some_mesh_data.mesh_file});
                    some_mesh_data.mesh_file = "";
                }
            });

            ui::DrawPanelComponent<PerspectiveCamera>("Camera", [&](){
                ui::DrawVec3UI("Position", camera_data.Position);
                ui::DrawVec3UI("Front", camera_data.Front);
            });


            /*
            for(auto[key, value] : m_SceneObjectLookup){
                ui::DrawPanelComponent<MeshComponent>(fmt::format("Object {}", key), [&](){
                    auto transform = *value->GetComponent<Transform>();
                    ui::DrawVec3UI(fmt::format("Object {} Pos", key), transform.Position);
                    ui::DrawVec3UI(fmt::format("Object {} Scale", key), transform.Scale);

                    // g_MeshFilepath
                    if(g_MeshFilepath != ""){
                        std::filesystem::path relative_path = std::filesystem::relative(g_MeshFilepath, "./");
                        // ConsoleLogTrace("in branch 2 mesh_file = {}", relative_path.filename().string());
                        value->SetComponent<MeshComponent>({g_MeshFilepath});
                        g_MeshFilepath = "";
                    }

                    value->SetComponent<Transform>({
                        .Position = transform.Position,
                        .Scale = transform.Scale
                    });
                });
            }
            */

            ImGui::End();
        }
    }

    //! TODO: Separate between OnUpdate tasks and OnRender-like tasks
    void LevelScene::OnUpdate() {
        // ConsoleLogTrace("LevelScene::OnUpdate");
        // ConsoleLogTrace("Controller Axis = {}", InputPoll::GetControllerAxis(0, 0));
        // if(InputPoll::IsControllerButtonPressed(1)){
        //     ConsoleLogTrace("Controller Button 0 Pressed!");
        // }

        auto camera_transform = *m_Camera->GetComponent<engine3d::Transform>();

        auto camera_comp = *m_Camera->GetComponent<engine3d::PerspectiveCamera>();

        //! TODO: Move DeltaTime out of global update
        //! TODO: GlobalUpdate just uses delta timer for frame-rate stuff, and we should move that in timer namespace
        //! TODO: Add a reset() function simply for resetting the fps, or anything time related
        //! TODO: By either doing timer::reset(dt) or doing timer::reset()
        //! @note Where reset that takes a parameter resets the delta time specified.
        //! @note And timer::reset with no params resets our timer automatically (though would effect the entire timing variables though unless we separate them some way)
        float deltaTime = sync_update::DeltaTime();

        
        if (InputPoll::IsKeyPressed(ENGINE_KEY_ESCAPE)){
            ApplicationInstance::GetWindow().Close();
        }

        if (InputPoll::IsKeyPressed(ENGINE_KEY_W)){
            camera_comp.ProcessKeyboard(FORWARD, deltaTime);
        }
        if (InputPoll::IsKeyPressed(ENGINE_KEY_S)){
            camera_comp.ProcessKeyboard(BACKWARD, deltaTime);
        }
        if (InputPoll::IsKeyPressed(ENGINE_KEY_A)){
            camera_comp.ProcessKeyboard(LEFT, deltaTime);
        }
        if (InputPoll::IsKeyPressed(ENGINE_KEY_D)){
            camera_comp.ProcessKeyboard(RIGHT, deltaTime);
        }
        if(InputPoll::IsKeyPressed(ENGINE_KEY_Q)){
            camera_comp.ProcessKeyboard(UP, deltaTime);
        }
        if(InputPoll::IsKeyPressed(ENGINE_KEY_E)){
            camera_comp.ProcessKeyboard(DOWN, deltaTime);
        }

        //! @note Press shift key to move using the mouse to rotate around
        if(InputPoll::IsKeyPressed(ENGINE_KEY_LEFT_SHIFT)){
            if(InputPoll::IsMousePressed(ENGINE_MOUSE_BUTTON_RIGHT)){
                glm::vec2 cursor_pos = InputPoll::GetMousePosition();

                float x_offset = cursor_pos.x;
                float velocity = x_offset * deltaTime;
                camera_comp.ProcessMouseMovement(-velocity, 0.f);
            }

            if(InputPoll::IsMousePressed(ENGINE_MOUSE_BUTTON_LEFT)){
                glm::vec2 cursor_pos = InputPoll::GetMousePosition();

                float x_offset = cursor_pos.x;
                float velocity = x_offset * deltaTime;
                camera_comp.ProcessMouseMovement(velocity, 0.f);
            }

            if(InputPoll::IsMousePressed(ENGINE_MOUSE_BUTTON_MIDDLE)){
                glm::vec2 cursor_pos = InputPoll::GetMousePosition();

                float velocity = cursor_pos.y * deltaTime;
                camera_comp.ProcessMouseMovement(0.f, velocity);
            }

            if(InputPoll::IsKeyPressed(ENGINE_KEY_SPACE)){
                // double xPosIn, yPosIn;
                // glfwGetCursorPos(ApplicationInstance::GetWindow().GetNativeWindow(), &xPosIn, &yPosIn);
                glm::vec2 cursor_pos = InputPoll::GetMousePosition();
                // float velocity = yPosIn * deltaTime;
                float velocity = cursor_pos.y * deltaTime;
                camera_comp.ProcessMouseMovement(0.f, -velocity);
            }
        }

        
        //! @note
        camera_comp.MovementSpeed = sensitivity;
        camera_comp.UpdateProjView();

        m_Camera->SetComponent<engine3d::PerspectiveCamera>(camera_comp);
        m_Camera->SetComponent<engine3d::Transform>(camera_transform);

        sphere_data.Position.x = glm::sin(sphere_data.Position.x);
        
        m_Sphere->SetComponent<Transform>({
            .Position = sphere_data.Position,
            .Rotation = {sphere_data.Rotation.x, sphere_data.Rotation.y, sphere_data.Rotation.z},
            .Scale = sphere_data.Scale,
        });

        m_Platform->SetComponent<Transform>({
            .Position = some_mesh_data.Position,
            .Rotation = some_mesh_data.Rotation,
            .Scale = some_mesh_data.Scale,
        });

        // Renderer::SetCamera(m_Camera);

        //! @note Solved the camera-jittering problem, by making each draw call be a submission call that those objects get called to render at the end of frame
        //! @note Rather then having them be rendered immediately as we update the properties relative to those objects
        // Renderer::RenderWithCamera(m_Sphere, m_Camera);
        // Renderer::SubmitSceneObject(m_Sphere);
        // Renderer::SubmitSceneObject(m_SomeMesh);
        // Renderer::RenderWithCamera(m_Sphere, m_Camera);
        // Renderer::RenderWithCamera(m_SomeMesh, m_Camera);

        // for(auto obj : m_MoreObjects){
        //     Renderer::RenderWithCamera(obj, m_Camera);
        // }

    }

    void LevelScene::OnSceneRender(){
        // SyncUpdateManager::DispatchOnUpdate();
        // ConsoleLogFatal("OnSceneRender Called!");
        Renderer::RenderWithCamera(m_Sphere, m_Camera);
        Renderer::RenderWithCamera(m_Platform, m_Camera);

        // for(auto obj : m_MoreObjects){
        //     Renderer::RenderWithCamera(obj, m_Camera);
        // }
    }
};


/*


RenderPass Attachments API

RenderPass rp = RenderPass();

// Or this should be defined at render pass initialization
rp.SetAttachments({
    {COLOR_FORMAT, sampleCount, NO_LOAD_STORE_STENCIL, NO_IMAGE_DEFINED}
});















*/


/*




[WorldTag]
ID = 1 (hash_id)


Actual Tag of WorldTag = "WorldTag#1"

- At creation generates/creates a specific UUID

flecs::world

[Scene1, 2, 3]
SystemRegistry::Register(world, this);



Scene Tag = "Tag#1"


VkFramebufferCreateInfo ci = {
    .depthStencil= VK_DEPTH_...
};


VkFramebuffer fb;


FramebufferSpecification spec = {
    .attachments = {
        COLOR_BUFFER_RBGA16
        DEPTH_STENCIL_READ_WRITE,
    }
};

FrameBuffer fb = Framebuffer(spec);

*/