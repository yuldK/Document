#pragma once

#include <limits>

namespace yul
{
	using line_type = size_t;
	using character_type = size_t;

	constexpr inline line_type max_line = std::numeric_limits<line_type>::max();
	constexpr inline character_type max_character = std::numeric_limits<character_type>::max();
}
