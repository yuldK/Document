#include <vector>
#include <fstream>
#include <algorithm>

#include "document.h"
#include "encoding.h"

namespace fs = std::filesystem;
using namespace yul;

std::unique_ptr<TextDocument> TextDocument::make(const std::filesystem::path& path)
{
	if (fs::exists(path) == false)
		return nullptr;

	std::ifstream stream{ path, std::ios::binary };
	if (stream.is_open() == false)
		return nullptr;

	auto size = fs::file_size(path);
	std::vector<std::byte> binary;
	binary.resize(size, {});

	stream.read(reinterpret_cast<char*>(binary.data()), size);

	auto document = make(binary);
	if (document)
		document->path_ = path;

	return document;
}

std::unique_ptr<TextDocument> TextDocument::make(std::span<std::byte> binary)
{
	auto document = std::unique_ptr<TextDocument>{ new TextDocument{} };

	// 인코딩 감지
	Encoding encoding = detectEncoding(binary);

	if (encoding == Encoding::utf8)
	{
		// UTF-8: BOM 제거 후 그대로 저장
		size_t offset = 0;
		if (checkBOM(binary))
			offset = 3; // BOM 크기

		auto dataSpan = binary.subspan(offset);
		document->text_ = std::u8string{
			reinterpret_cast<const char8_t*>(dataSpan.data()),
			reinterpret_cast<const char8_t*>(dataSpan.data() + dataSpan.size())
		};
	}
	else if (encoding == Encoding::euckr)
	{
		// EUC-KR: UTF-8로 변환
		document->text_ = convertEucKrToUtf8(binary);
	}
	else
	{
		// Unknown: UTF-8로 간주 (fallback)
		document->text_ = std::u8string{
			reinterpret_cast<const char8_t*>(binary.data()),
			reinterpret_cast<const char8_t*>(binary.data() + binary.size())
		};
	}

	document->build();
	return document;
}

bool TextDocument::build()
{
	lineOffsets_.clear();
	lineOffsets_.push_back(0); // 첫 번째 줄은 항상 offset 0에서 시작

	// text_를 순회하며 줄바꿈 문자를 찾아 다음 줄의 시작 offset을 기록
	// 지원하는 줄바꿈: \n, \r\n, \r
	for (size_t i = 0; i < text_.size(); ++i)
	{
		char8_t ch = text_[i];

		if (ch == u8'\r')
		{
			// \r\n인지 확인
			if (i + 1 < text_.size() && text_[i + 1] == u8'\n')
			{
				// \r\n: 다음 줄은 i + 2부터 시작
				lineOffsets_.push_back(i + 2);
				++i; // \n을 건너뛰기
			}
			else
			{
				// \r만 있는 경우: 다음 줄은 i + 1부터 시작
				lineOffsets_.push_back(i + 1);
			}
		}
		else if (ch == u8'\n')
		{
			// \n: 다음 줄은 i+1부터 시작
			lineOffsets_.push_back(i + 1);
		}
	}

	return true;
}

Position TextDocument::positionAt(size_t offset) const
{
	// offset이 텍스트 범위를 벗어난 경우는 끝 위치 반환
	if (offset >= text_.size())
	{
		if (lineOffsets_.size() == 1)
			return Position{ 0, text_.size() };

		size_t lastLine = lineOffsets_.size() - 1; // 마지막 줄 인덱스
		size_t lastLineStart = lineOffsets_[lastLine];
		return Position{ lastLine, text_.size() - lastLineStart };
	}

	// 이진 검색으로 offset이 속한 줄 찾기
	// lower_bound: offset보다 크거나 같은 첫 번째 요소를 찾음
	auto it = std::lower_bound(lineOffsets_.begin(), lineOffsets_.end(), offset + 1);

	// it는 offset + 1보다 크거나 같은 첫 번째 줄의 시작 offset을 가리킴
	// 실제 줄의 인덱스는 it - 1
	size_t line = std::distance(lineOffsets_.begin(), it) - 1;
	size_t lineStart = lineOffsets_[line];
	size_t character = offset - lineStart;

	return Position{ line, character };
}

size_t TextDocument::offsetAt(const Position& position) const
{
	// 줄 번호가 범위를 벗어난 텍스트 끝 반환
	if (position.line + 1 >= lineOffsets_.size())
		return text_.size();

	size_t lineStart = lineOffsets_[position.line];
	size_t lineEnd = lineOffsets_[position.line + 1];

	// character가 줄 길이를 벗어난 줄 끝 반환
	size_t offset = lineStart + position.character;
	if (offset > lineEnd)
		return lineEnd;

	return offset;
}

std::u8string_view TextDocument::getText() const
{
	return std::u8string_view{ text_ };
}

std::u8string_view TextDocument::getText(const Range& range) const
{
	size_t start = offsetAt(range.begin());
	size_t end = offsetAt(range.end());

	return getText(start, end);
}

std::u8string_view TextDocument::getText(size_t start, size_t end) const
{
	if (start >= text_.size())
		return std::u8string_view{};

	if (end > text_.size())
		end = text_.size();

	if (start >= end)
		return std::u8string_view{};

	return std::u8string_view{ text_.data() + start, end - start };
}

std::u8string_view TextDocument::getLine(line_type line) const
{
	if (line >= lineOffsets_.size())
		return std::u8string_view{};

	auto start = lineOffsets_[line];
	auto end = (line + 1 == lineOffsets_.size()) ? text_.length() : lineOffsets_[line + 1];
	if (end > start && text_[end - 1] == u8'\n')
		--end;
	if (end > start && text_[end - 1] == u8'\r')
		--end;

	return getText(start, end);
}

line_type TextDocument::lineLength() const
{
	return lineOffsets_.size();
}
