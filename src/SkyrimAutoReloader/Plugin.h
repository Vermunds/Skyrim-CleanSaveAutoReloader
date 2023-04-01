#pragma once

#include <string_view>

namespace Plugin
{
	using namespace std::literals;

	inline constexpr REL::Version VERSION
	{
		// clang-format off
		1u,
		0u,
		0u,
		// clang-format on
	};
	
	inline constexpr auto VERSION_STRING = "v1.0.0"sv;

	inline constexpr auto NAME = "SkyrimAutoReloader"sv;
}
