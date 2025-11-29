#pragma once

#include "type.h"
#include "position.h"
#include "range.h"
#include <memory>
#include <string>
#include <string_view>

#include <filesystem>
#include <span>

class Document
{
public:

	// factory pattern
public:
	static std::unique_ptr<Document> make(std::filesystem::path path);
	static std::unique_ptr<Document> make(std::span<std::byte> binary);


private:
	std::filesystem::path path_;
	std::u8string text_;
};

