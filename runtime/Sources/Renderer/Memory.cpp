#include <mlx_profile.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION 1000000
#define VMA_ASSERT(expr) ((void)0)
#define VMA_IMPLEMENTATION

#ifdef MLX_COMPILER_CLANG
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Weverything"
		#include <PreCompiled.h>
	#pragma clang diagnostic pop
#elif defined(MLX_COMPILER_GCC)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
	#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	#pragma GCC diagnostic ignored "-Wunused-variable"
	#pragma GCC diagnostic ignored "-Wparentheses"
		#include <PreCompiled.h>
	#pragma GCC diagnostic pop
#else
	#include <PreCompiled.h>
#endif

#include <Renderer/RenderCore.h>

namespace mlx
{
	void GPUAllocator::Init() noexcept
	{
		VmaVulkanFunctions vma_vulkan_func{};
		vma_vulkan_func.vkAllocateMemory                    = vkAllocateMemory;
		vma_vulkan_func.vkBindBufferMemory                  = vkBindBufferMemory;
		vma_vulkan_func.vkBindImageMemory                   = vkBindImageMemory;
		vma_vulkan_func.vkCreateBuffer                      = vkCreateBuffer;
		vma_vulkan_func.vkCreateImage                       = vkCreateImage;
		vma_vulkan_func.vkDestroyBuffer                     = vkDestroyBuffer;
		vma_vulkan_func.vkDestroyImage                      = vkDestroyImage;
		vma_vulkan_func.vkFlushMappedMemoryRanges           = vkFlushMappedMemoryRanges;
		vma_vulkan_func.vkFreeMemory                        = vkFreeMemory;
		vma_vulkan_func.vkGetBufferMemoryRequirements       = vkGetBufferMemoryRequirements;
		vma_vulkan_func.vkGetImageMemoryRequirements        = vkGetImageMemoryRequirements;
		vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
		vma_vulkan_func.vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties;
		vma_vulkan_func.vkInvalidateMappedMemoryRanges      = vkInvalidateMappedMemoryRanges;
		vma_vulkan_func.vkMapMemory                         = vkMapMemory;
		vma_vulkan_func.vkUnmapMemory                       = vkUnmapMemory;
		vma_vulkan_func.vkCmdCopyBuffer                     = vkCmdCopyBuffer;

		VmaAllocatorCreateInfo allocator_create_info{};
		allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_0;
		allocator_create_info.physicalDevice = RenderCore::Get().GetPhysicalDevice();
		allocator_create_info.device = RenderCore::Get().GetDevice();
		allocator_create_info.instance = RenderCore::Get().GetInstance();
		allocator_create_info.pVulkanFunctions = &vma_vulkan_func;

		kvfCheckVk(vmaCreateAllocator(&allocator_create_info, &m_allocator));
		DebugLog("Graphics allocator : created new allocator");
	}

	VmaAllocation GPUAllocator::CreateBuffer(const VkBufferCreateInfo* binfo, const VmaAllocationCreateInfo* vinfo, VkBuffer& buffer, const char* name) noexcept
	{
		MLX_PROFILE_FUNCTION();
		VmaAllocation allocation;
		kvfCheckVk(vmaCreateBuffer(m_allocator, binfo, vinfo, &buffer, &allocation, nullptr));
		if(name != nullptr)
		{
			vmaSetAllocationName(m_allocator, allocation, name);
		}
		DebugLog("Graphics Allocator : created new buffer '%'", name);
		m_active_buffers_allocations++;
		return allocation;
	}

	void GPUAllocator::DestroyBuffer(VmaAllocation allocation, VkBuffer buffer) noexcept
	{
		MLX_PROFILE_FUNCTION();
		RenderCore::Get().WaitDeviceIdle();
		vmaDestroyBuffer(m_allocator, buffer, allocation);
		DebugLog("Graphics Allocator : destroyed buffer");
		m_active_buffers_allocations--;
	}

	VmaAllocation GPUAllocator::CreateImage(const VkImageCreateInfo* iminfo, const VmaAllocationCreateInfo* vinfo, VkImage& image, const char* name) noexcept
	{
		MLX_PROFILE_FUNCTION();
		VmaAllocation allocation;
		kvfCheckVk(vmaCreateImage(m_allocator, iminfo, vinfo, &image, &allocation, nullptr));
		if(name != nullptr)
		{
			vmaSetAllocationName(m_allocator, allocation, name);
		}
		DebugLog("Graphics Allocator : created new image '%'", name);
		m_active_images_allocations++;
		return allocation;
	}

	void GPUAllocator::DestroyImage(VmaAllocation allocation, VkImage image) noexcept
	{
		MLX_PROFILE_FUNCTION();
		RenderCore::Get().WaitDeviceIdle();
		vmaDestroyImage(m_allocator, image, allocation);
		DebugLog("Graphics Allocator : destroyed image");
		m_active_images_allocations--;
	}

	void GPUAllocator::MapMemory(VmaAllocation allocation, void** data) noexcept
	{
		MLX_PROFILE_FUNCTION();
		kvfCheckVk(vmaMapMemory(m_allocator, allocation, data));
	}

	void GPUAllocator::UnmapMemory(VmaAllocation allocation) noexcept
	{
		MLX_PROFILE_FUNCTION();
		vmaUnmapMemory(m_allocator, allocation);
	}

	void GPUAllocator::DumpMemoryToJson()
	{
		static std::uint32_t id = 0;
		std::string name("memory_dump");
		name.append(std::to_string(id) + ".json");
		std::ofstream file(name);
		if(!file.is_open())
		{
			Error("Graphics allocator : unable to dump memory to a json file");
			return;
		}
		char* str = nullptr;
		vmaBuildStatsString(m_allocator, &str, true);
			file << str;
		vmaFreeStatsString(m_allocator, str);
		file.close();
		id++;
	}

	void GPUAllocator::Flush(VmaAllocation allocation, VkDeviceSize size, VkDeviceSize offset) noexcept
	{
		MLX_PROFILE_FUNCTION();
		vmaFlushAllocation(m_allocator, allocation, offset, size);
	}

	void GPUAllocator::Destroy() noexcept
	{
		if(m_active_images_allocations != 0)
			Error("Graphics allocator : some user-dependant allocations were not freed before destroying the display (% active allocations). You may have not destroyed all the MLX resources you've created", m_active_images_allocations);
		else if(m_active_buffers_allocations != 0)
			Error("Graphics allocator : some MLX-dependant allocations were not freed before destroying the display (% active allocations). This is an error in the MLX, please report this should not happen", m_active_buffers_allocations);
		if(m_active_images_allocations < 0 || m_active_buffers_allocations < 0)
			Warning("Graphics allocator : the impossible happened, the MLX has freed more allocations than it has made (wtf)");
		vmaDestroyAllocator(m_allocator);
		m_active_buffers_allocations = 0;
		m_active_images_allocations = 0;
		DebugLog("Vulkan : destroyed a graphics allocator");
	}
}