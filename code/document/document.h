#pragma once

#include "type.h"
#include "position.h"
#include "range.h"

#include <memory>
#include <vector>

#include <string>
#include <string_view>

#include <span>
#include <filesystem>

namespace yul
{
	class TextDocument
	{
	public:

		// factory pattern
	public:
		static std::unique_ptr<TextDocument> make(const std::filesystem::path& path);
		static std::unique_ptr<TextDocument> make(std::span<std::byte> binary);

	public:
		// offset(0-based)을 Position으로 변환
		Position positionAt(size_t offset) const;

		// Position을 offset(0-based)으로 변환
		size_t offsetAt(const Position& position) const;

		std::u8string_view getText() const;
		std::u8string_view getText(const Range& range) const;
		std::u8string_view getText(size_t start, size_t end) const;

		std::u8string_view getLine(line_type line) const;
		line_type lineLength() const;
	private:
		bool build();

	private:
		TextDocument() = default;

	private:
		std::optional<std::filesystem::path> path_ = std::nullopt;
		std::u8string text_;

		// 각 줄의 시작 offset을 저장 (0-based)
		// lineOffsets_[0] = 0 (첫 번째 줄)
		// lineOffsets_[n] = n번째 줄의 시작 offset
		std::vector<size_t> lineOffsets_;
	};
}
