/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maldavid <kbz_8.dev@akel-engine.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/04 21:53:12 by maldavid          #+#    #+#             */
/*   Updated: 2024/05/25 16:11:00 by maldavid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __MLX_WINDOW__
#define __MLX_WINDOW__

namespace mlx
{
	class Window
	{
		public:
			Window(std::size_t w, std::size_t h, const std::string& title);

			inline void* GetWindowHandle() const noexcept { return p_window; }
			inline int GetWidth() const noexcept { return m_width; }
			inline int GetHeight() const noexcept { return m_height; }
			inline std::uint32_t GetID() const noexcept { return m_id; }

			void Destroy() noexcept;

			~Window() = default;

		private:
			void* p_window = nullptr;
			std::uint32_t m_id = -1;
			int m_width = 0;
			int m_height = 0;
	};
}

#endif