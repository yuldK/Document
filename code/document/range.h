#pragma once

#include "type.h"
#include "position.h"

namespace yul
{
	// [, )
	struct Range
	{
	private:
		Position begin_;
		Position end_;

	public:
		constexpr Position begin() const noexcept { return begin_; }
		constexpr Position end() const noexcept { return end_; }

	public:
		constexpr Range(Position begin, Position end) noexcept
			: begin_{ std::min(begin, end) }
			, end_{ std::max(begin, end) }
		{
		}

		constexpr Range(line_type bl, character_type bc, line_type el, character_type ec) noexcept
			: Range(Position{ bl, bc }, Position{ el, ec })
		{
		}

		constexpr Range& operator=(const Range& other) noexcept
		{
			begin_ = other.begin_;
			end_ = other.end_;
			return *this;
		}

		constexpr Range& operator=(Range&& other) noexcept
		{
			*this = other;
			// other 비우기
			other = Range{ {}, {} };
			return *this;
		}

		constexpr bool operator==(const Range& other) const noexcept
		{
			return begin_ == other.begin_ && end_ == other.end_;
		}

		constexpr auto operator<=>(const Range& other) const noexcept
		{
			if (other.end_ < begin_)
				return std::partial_ordering::less;
			if (end_ < other.begin_)
				return std::partial_ordering::greater;
			if (begin_ == other.begin_ && end_ == other.end_)
				return std::partial_ordering::equivalent;
			return std::partial_ordering::unordered;
		}

		constexpr bool in(const Position& pos, bool includeEnd = false) const noexcept
		{
			return begin_ <= pos && (pos < end_ || (includeEnd ? pos == end_ : false));
		}

		constexpr bool in(const Range& other, bool includeEnd = false) const noexcept
		{
			return in(other.begin_, includeEnd) && in(other.end_, includeEnd);
		}

		constexpr bool intersect(const Range& other, bool includeEnd = false) const noexcept
		{
			return in(other.begin_, includeEnd) != in(other.end_, includeEnd);
		}
	};

	// static test
	static_assert(Range{ 5, 2, 5, 5 } == Range{ 5, 2, 5, 5 }, "Range == operator not verified!");
}
