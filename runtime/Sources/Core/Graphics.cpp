#include <PreCompiled.h>
#include <Core/Graphics.h>

namespace mlx
{
	GraphicsSupport::GraphicsSupport(std::size_t w, std::size_t h, NonOwningPtr<Texture> render_target, int id) :
		p_window(nullptr),
		m_width(w),
		m_height(h),
		m_id(id),
		m_has_window(false)
	{
		MLX_PROFILE_FUNCTION();
		m_renderer.SetWindow(nullptr);
		m_renderer.Init(render_target);
		m_scene_renderer.Init();

		SceneDescriptor descriptor{};
		descriptor.renderer = &m_renderer;
		p_scene = std::make_unique<Scene>(std::move(descriptor));
	}

	GraphicsSupport::GraphicsSupport(std::size_t w, std::size_t h, std::string title, int id) :
		p_window(std::make_shared<Window>(w, h, title)),
		m_width(w),
		m_height(h),
		m_id(id),
		m_has_window(true)
	{
		MLX_PROFILE_FUNCTION();
		m_renderer.SetWindow(p_window.get());
		m_renderer.Init(nullptr);
		m_scene_renderer.Init();

		SceneDescriptor descriptor{};
		descriptor.renderer = &m_renderer;
		p_scene = std::make_unique<Scene>(std::move(descriptor));
	}

	void GraphicsSupport::Render() noexcept
	{
		MLX_PROFILE_FUNCTION();
		if(m_renderer.BeginFrame())
		{
			m_scene_renderer.Render(*p_scene, m_renderer);
			m_renderer.EndFrame();
		}

		#ifdef GRAPHICS_MEMORY_DUMP
			// dump memory to file every two seconds
			using namespace std::chrono_literals;
			static std::int64_t timer = static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
			if(std::chrono::duration<std::uint64_t>{static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()) - timer} >= 1s)
			{
				RenderCore::Get().GetAllocator().DumpMemoryToJson();
				timer = static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
			}
		#endif
	}

	GraphicsSupport::~GraphicsSupport()
	{
		MLX_PROFILE_FUNCTION();
		RenderCore::Get().WaitDeviceIdle();
		p_scene.reset();
		m_scene_renderer.Destroy();
		m_renderer->Destroy();
		if(p_window)
			p_window->Destroy();
	}
}