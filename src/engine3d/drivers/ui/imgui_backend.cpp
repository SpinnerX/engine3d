#include "drivers/vulkan/helper_functions.hpp"
#include "drivers/vulkan/vulkan_core.hpp"
#include "drivers/vulkan/vulkan_renderer.hpp"
#include "engine_logger.hpp"
#include <drivers/ui/imgui_backend.hpp>
#include <core/application_instance.hpp>
#include <drivers/vulkan/vulkan_context.hpp>
#include <imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <fmt/format.h>

// constexpr void assert_expression(bool expression, char* message){
// 	// static constexpr auto* name = "";
// 	static_assert(expression, "name");
// }

// constexpr void assert_expression(){
// 	constexpr std::string_view message = "This is my error message";

// 	static_assert(false, message);
// }

namespace engine3d{
	static void ImGuiLayoutColorModification(){
		auto& colors = ImGui::GetStyle().Colors; // @note Colors is ImVec4
			
			colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

			// Headers
			colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
			colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
			colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
			
			
			// Buttons
			colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
			colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
			colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };


			// Frame BG
			colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
			colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
			colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };


			// Tabs
			colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
			colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
			colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
			colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.15f, 1.0f };
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
			
			// Titles
			colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
			colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.15f, 1.0f };
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.1f, 0.150f, 0.951f, 1.0f };
	}
	
	static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;
	// static VkCommandPool imgui_command_pool;
	static VkDescriptorPool s_DescriptorPool;
	// static VkRenderPass imgui_renderpass;
	// static std::vector<VkFramebuffer> s_ImGuiFramebuffers;
	// static std::vector<VkImageView> s_ImGuiViewportImageViews;
	// static std::vector<VkImage> s_ImGuiViewportImages;
	static std::vector<VkFramebuffer> s_ViewportFramebuffers;
	VkCommandPool s_ViewportCommandPool;
	VkRenderPass s_ViewportRenderpass;

	struct ImGuiImage{
		VkImage Image;
		VkImageView ImageView;
		VkDeviceMemory ImageDeviceMemory;
	};

	static std::vector<ImGuiImage> s_ImGuiViewportImages;

	// static std::vector<
	// static VkCommandPool cmd_pool;

    void ImGuiBackend::Initialize(){
        ConsoleLogInfo("ImGui Vulkan Test: Begin Initialization!");

		// 1: create descriptor pool for IMGUI
		//  the size of the pool is very oversize, but it's copied from imgui demo
		//  itself.
		VkDescriptorPoolSize pool_sizes[] = { 
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
		};

		VkDescriptorPoolCreateInfo desc_pool_create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes),
			// .poolSizeCount = (uint32_t)std::size(pool_sizes),
			.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes),
			.pPoolSizes = pool_sizes
		};

		// VkDescriptorPool imgui_pool;
		vk::vk_check(vkCreateDescriptorPool(vk::VulkanContext::GetDriver(), &desc_pool_create_info, nullptr, &s_DescriptorPool), "vkCreateDescriptorPool", __FILE__, __LINE__, __FUNCTION__);
		ConsoleLogInfo("After creating descriptor sets for IMGUI");



		//! @note Creating separate render pass for imgui
		VkAttachmentDescription attachment_description = {
			.format = ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetSwapchainFormat(),
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentReference color_attachment = {
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkSubpassDescription subpass = {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &color_attachment
		};

		VkSubpassDependency subpass_dependency = {
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0, // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		};

		VkRenderPassCreateInfo imgui_renderpass_ci = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments = &attachment_description,
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = &subpass_dependency
		};

		vk::vk_check(vkCreateRenderPass(vk::VulkanContext::GetDriver(), &imgui_renderpass_ci, nullptr, &s_ViewportRenderpass), "vkCreateRenderPass", __FILE__, __LINE__, __FUNCTION__);

		ConsoleLogWarn("After Creating Renderpass!");

		// static_assert(false, "Error from static_assert!");
		// assert(true);
		// auto swapchain_images_size = ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetImagesSize();
		
		auto swapchain = ApplicationInstance::GetWindow().GetCurrentSwapchain();
		auto swapchain_images_size = swapchain->GetImagesSize();

		// Creating viewport framebuffers
		s_ViewportFramebuffers.resize(swapchain_images_size);
		ConsoleLogWarn("Resize viewport framebuffers = {}", s_ViewportFramebuffers.size());

		for(size_t i = 0; i < s_ViewportFramebuffers.size(); i++){
			VkImageView attachment[1];
			//! @note For Framebuffer abstraction layer have the specifications
			//! @note Each framebuffer abstraction will define the renderpass themselves
			VkFramebufferCreateInfo fb_ci = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = s_ViewportRenderpass,
				.attachmentCount = 1,
				.pAttachments = attachment,
				.width = ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetSwapchainExtent().width,
				.height = ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetSwapchainExtent().height,
				.layers = 1,
			};

			vk::vk_check(vkCreateFramebuffer(vk::VulkanContext::GetDriver(), &fb_ci, nullptr, &s_ViewportFramebuffers[i]), "vkCreateFramebuffer", __FILE__, __LINE__, __FUNCTION__);
		}

		ConsoleLogWarn("After setting our framebuffers information. Framebuffer.size() = {}", s_ViewportFramebuffers.size());
#if 0
		for(size_t i = 0; i <  s_ImGuiViewportImageViews.size(); i++){
			VkImageCreateInfo image_view_ci = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType = VK_IMAGE_TYPE_2D,
				// Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
				.format = VK_FORMAT_B8G8R8A8_SRGB,
				// .extent.width = swapchain->GetSwapchainExtent().width,
				// .extent.height = swapchain->GetSwapchainExtent().height,
				.extent = {
					.width = swapchain->GetSwapchainExtent().width,
					.height = swapchain->GetSwapchainExtent().height,
					.depth = 1
				},
				// .extent.depth = 1,
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.tiling = VK_IMAGE_TILING_LINEAR,
				.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			};

			vk::vk_check(vkCreateImageView(vk::VulkanContext::GetDriver(), &image_view_ci, nullptr, ))
		}
#endif

		//! @note Creating images for the viewport
		s_ImGuiViewportImages.resize(swapchain_images_size);


		ConsoleLogTrace("After setting viewport images.size() = {}", s_ImGuiViewportImages.size());
		//! @note Viewport-specific images
		for(size_t i = 0; i <  s_ImGuiViewportImages.size(); i++){
			s_ImGuiViewportImages[i].Image = vk::CreateImage(VK_FORMAT_B8G8R8A8_SRGB, swapchain->GetSwapchainExtent().width, swapchain->GetSwapchainExtent().height);

			// Create memory to backup image
			VkMemoryRequirements memory_requirements;
			vkGetImageMemoryRequirements(vk::VulkanContext::GetDriver(), s_ImGuiViewportImages[i].Image, &memory_requirements);
			
			// Allocate memory for these images
			VkMemoryAllocateInfo mem_alloc_info = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
			};
			vkAllocateMemory(vk::VulkanContext::GetDriver(), &mem_alloc_info, nullptr, &s_ImGuiViewportImages[i].ImageDeviceMemory);
			vkBindImageMemory(vk::VulkanContext::GetDriver(), s_ImGuiViewportImages[i].Image, s_ImGuiViewportImages[i].ImageDeviceMemory, 0);

			// VkCommandBuffer copy_command_buffer = vk::BeginSingleTimeCommand(s_ViewportCommandPool);
			// vk::PipelineImageBarrier(copy_command_buffer, s_ImGuiViewportImages[i].Image, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
			// vk::EndSingleTimeCommand(copy_command_buffer, s_ViewportCommandPool);
		}



		//! @note Setting up imgui stuff.
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		// Setting custom dark themed imgui layout
		ImGuiLayoutColorModification();

		// Setup Dear ImGui style
		// ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();
		// Setup Dear ImGui style

        ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// this initializes imgui for Vulkan
		ImGui_ImplGlfw_InitForVulkan(ApplicationInstance::GetWindow().GetNativeWindow(), true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = vk::VulkanContext::GetVkInstance();
		init_info.PhysicalDevice = vk::VulkanContext::GetPhysicalDriver();
		init_info.Device = vk::VulkanContext::GetDriver();
		init_info.Queue = vk::VulkanContext::GetDriver().GetGraphicsQueue();
		init_info.RenderPass = ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetRenderPass();
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = s_DescriptorPool;
		init_info.MinImageCount = 2;
		init_info.ImageCount = ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetImagesSize();
		init_info.UseDynamicRendering = false;




		//dynamic rendering parameters for imgui to use
		// init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		// init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		// init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = (const VkFormat*)ApplicationInstance::GetWindow().GetCurrentSwapchain()->GetSwapchainFormat();

		// init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		ImGui_ImplVulkan_Init(&init_info);
		ConsoleLogWarn("After ImGui_ImplGlfw_InitForVulkan called!");

		ImGui_ImplVulkan_CreateFontsTexture();

	}

	//! TODO: Get scene into imgui window
    void ImGuiBackend::Begin(){
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

    void ImGuiBackend::End(){
		ImGui::Render();

		auto current_cmd_buffer = vk::VulkanRenderer::CurrentCommandBuffer();

		ImDrawData* draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(draw_data, current_cmd_buffer);

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
};