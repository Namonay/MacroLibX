#ifndef __MLX_RENDERER_ENUMS__
#define __MLX_RENDERER_ENUMS__

namespace mlx
{
	enum class BufferType
	{
		Constant = 0,
		Staging,
		HighDynamic, // typically stored in RAM
		LowDynamic,  // typically stored in VRAM

		EndEnum
	};
	constexpr std::size_t BufferTypeCount = static_cast<std::size_t>(BufferType::EndEnum);

	enum class ImageType
	{
		Color = 0,
		Depth,

		EndEnum
	};
	constexpr std::size_t ImageTypeCount = static_cast<std::size_t>(ImageType::EndEnum);
}

#endif