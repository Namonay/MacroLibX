#include <PreCompiled.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderCore.h>
#include <Core/Enums.h>
#include <Renderer/Pipelines/Shader.h>
#include <Core/EventBus.h>

namespace mlx
{
	namespace Internal
	{
		struct ResizeEventBroadcast : public EventBase
		{
			Event What() const override { return Event::ResizeEventCode; }
		};

		struct FrameBeginEventBroadcast : public EventBase
		{
			Event What() const override { return Event::FrameBeginEventCode; }
		};

		struct DescriptorPoolResetEventBroadcast : public EventBase
		{
			Event What() const override { return Event::DescriptorPoolResetEventCode; }
		};
	}

	void Renderer::Init(NonOwningPtr<Window> window)
	{
		func::function<void(const EventBase&)> functor = [this](const EventBase& event)
		{
			if(event.What() == Event::ResizeEventCode)
				this->RequireFramebufferResize();
		};
		EventBus::RegisterListener({ functor, "__ScopRenderer" });

		p_window = window;

		auto& render_core = RenderCore::Get();
		m_surface = p_window->CreateVulkanSurface(render_core::GetInstance());
		DebugLog("Vulkan : surface created");

		CreateSwapchain();

		for(std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_image_available_semaphores[i] = kvfCreateSemaphore(render_core.GetDevice());
			DebugLog("Vulkan : image available semaphore created");
			m_render_finished_semaphores[i] = kvfCreateSemaphore(render_core.GetDevice());
			DebugLog("Vulkan : render finished semaphore created");
			m_cmd_buffers[i] = kvfCreateCommandBuffer(render_core.GetDevice());
			DebugLog("Vulkan : command buffer created");
			m_cmd_fences[i] = kvfCreateFence(render_core.GetDevice());
			DebugLog("Vulkan : fence created");
		}
	}

	bool Renderer::BeginFrame()
	{
		kvfWaitForFence(RenderCore::Get().GetDevice(), m_cmd_fences[m_current_frame_index]);
		VkResult result = vkAcquireNextImageKHR(RenderCore::Get().GetDevice(), m_swapchain, UINT64_MAX, m_image_available_semaphores[m_current_frame_index], VK_NULL_HANDLE, &m_swapchain_image_index);
		if(result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			DestroySwapchain();
			CreateSwapchain();
			EventBus::SendBroadcast(Internal::ResizeEventBroadcast{});
			return false;
		}
		else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			FatalError("Vulkan error : failed to acquire swapchain image, %", kvfVerbaliseVkResult(result));

		vkResetCommandBuffer(m_cmd_buffers[m_current_frame_index], 0);
		kvfBeginCommandBuffer(m_cmd_buffers[m_current_frame_index], 0);
		m_drawcalls = 0;
		m_polygons_drawn = 0;
		EventBus::SendBroadcast(Internal::FrameBeginEventBroadcast{});
		return true;
	}

	void Renderer::EndFrame()
	{
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		kvfEndCommandBuffer(m_cmd_buffers[m_current_frame_index]);
		kvfSubmitCommandBuffer(RenderCore::Get().GetDevice(), m_cmd_buffers[m_current_frame_index], KVF_GRAPHICS_QUEUE, m_render_finished_semaphores[m_current_frame_index], m_image_available_semaphores[m_current_frame_index], m_cmd_fences[m_current_frame_index], wait_stages);
		if(!kvfQueuePresentKHR(RenderCore::Get().GetDevice(), m_render_finished_semaphores[m_current_frame_index], m_swapchain, m_swapchain_image_index) || m_framebuffers_resize)
		{
			m_framebuffers_resize = false;
			DestroySwapchain();
			CreateSwapchain();
			EventBus::SendBroadcast(Internal::ResizeEventBroadcast{});
		}
		m_current_frame_index = (m_current_frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
		kvfResetDeviceDescriptorPools(RenderCore::Get().GetDevice());
		EventBus::SendBroadcast(Internal::DescriptorPoolResetEventBroadcast{});
	}

	void Renderer::CreateSwapchain()
	{
		Vec2ui drawable_size = p_window->GetVulkanDrawableSize();
		VkExtent2D extent = { drawable_size.x, drawable_size.y };
		m_swapchain = kvfCreateSwapchainKHR(RenderCore::Get().GetDevice(), RenderCore::Get().GetPhysicalDevice(), m_surface, extent, false);

		std::uint32_t images_count = kvfGetSwapchainImagesCount(m_swapchain);
		std::vector<VkImage> tmp(images_count);
		m_swapchain_images.resize(images_count);
		vkGetSwapchainImagesKHR(RenderCore::Get().GetDevice(), m_swapchain, &images_count, tmp.data());
		for(std::size_t i = 0; i < images_count; i++)
		{
			m_swapchain_images[i].Init(tmp[i], kvfGetSwapchainImagesFormat(m_swapchain), extent.width, extent.height);
			m_swapchain_images[i].TransitionLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			m_swapchain_images[i].CreateImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
		}
		DebugLog("Vulkan : swapchain created");
	}

	void Renderer::DestroySwapchain()
	{
		RenderCore::Get().WaitDeviceIdle();
		for(Image& img : m_swapchain_images)
			img.DestroyImageView();
		kvfDestroySwapchainKHR(RenderCore::Get().GetDevice(), m_swapchain);
		DebugLog("Vulkan : swapchain destroyed");
	}

	void Renderer::Destroy() noexcept
	{
		auto& render_core = RenderCore::Get();
		render_core.WaitDeviceIdle();

		for(std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			kvfDestroySemaphore(render_core.GetDevice(), m_image_available_semaphores[i]);
			DebugLog("Vulkan : image available semaphore destroyed");
			kvfDestroySemaphore(render_core.GetDevice(), m_render_finished_semaphores[i]);
			DebugLog("Vulkan : render finished semaphore destroyed");
			kvfDestroyFence(render_core.GetDevice(), m_cmd_fences[i]);
			DebugLog("Vulkan : fence destroyed");
		}

		DestroySwapchain();
		vkDestroySurfaceKHR(render_core.GetInstance(), m_surface, nullptr);
		DebugLog("Vulkan : surface destroyed");
		m_surface = VK_NULL_HANDLE;
	}
}