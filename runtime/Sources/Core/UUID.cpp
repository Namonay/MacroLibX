/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UUID.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maldavid <kbz_8.dev@akel-engine.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 11:26:37 by maldavid          #+#    #+#             */
/*   Updated: 2024/04/23 14:09:35 by maldavid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <pre_compiled.h>

#include <core/UUID.h>

namespace mlx
{
	static std::random_device random_device;
	static std::mt19937_64 engine(random_device());
	static std::uniform_int_distribution<std::uint64_t> uniform_distribution;

	UUID::UUID() : m_uuid(uniform_distribution(engine)) {}
	UUID::UUID(std::uint64_t uuid) : m_uuid(uuid) {}
}