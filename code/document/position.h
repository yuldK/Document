#pragma once

#include <compare>
#include "type.h"

namespace yul
{
	// line, character
	struct Position
	{
		line_type line = max_line;
		character_type character = max_character;

		constexpr auto operator<=>(const Position&) const noexcept = default;
	};

	// static test
	static_assert(Position{ 5, 1 } > Position{ 4, 1 }, "Position > operator not verified!");
	static_assert(Position{ 5, 1 } < Position{ 5, 2 }, "Position < operator not verified!");
	static_assert(Position{ 5, 2 } == Position{ 5, 2 }, "Position == operator not verified!");
}
