/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maldavid <kbz_8.dev@akel-engine.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/04 17:36:44 by maldavid          #+#    #+#             */
/*   Updated: 2024/07/05 13:12:51 by maldavid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <PreCompiled.h>

#include <Core/SDLManager.h>
#include <Platform/Window.h>

namespace mlx
{
	Window::Window(std::size_t w, std::size_t h, const std::string& title) : m_width(w), m_height(h)
	{
		static std::uint64_t ids = 0;
		p_window = SDLManager::Get().CreateWindow(title, w, h);
		m_id = ids++;
	}

	void Window::Destroy() noexcept
	{
		if(p_window != nullptr)
		{
			SDLManager::Get().DestroyWindow(p_window);
			p_window = nullptr;
		}
	}
}