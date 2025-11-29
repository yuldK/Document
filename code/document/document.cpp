#include <vector>
#include <fstream>

#include "document.h"
#include "encoding.h"

namespace fs = std::filesystem;

std::unique_ptr<Document> Document::make(fs::path path)
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

	return make(binary);
}

std::unique_ptr<Document> Document::make(std::span<std::byte> binary)
{
	auto document = std::unique_ptr<Document>{ new Document{} };

	// 인코딩 감지
	Encoding encoding = detectEncoding(binary);

	if (encoding == Encoding::UTF8)
	{
		// UTF-8: BOM 제거 후 그대로 사용
		size_t offset = 0;
		if (checkBOM(binary))
			offset = 3; // BOM 크기

		auto dataSpan = binary.subspan(offset);
		document->text_ = std::u8string{
			reinterpret_cast<const char8_t*>(dataSpan.data()),
			reinterpret_cast<const char8_t*>(dataSpan.data() + dataSpan.size())
		};
	}
	else if (encoding == Encoding::EUC_KR)
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

	return document;
}
