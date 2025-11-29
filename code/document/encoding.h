#pragma once

#include <span>
#include <string>

// 인코딩 타입
enum class Encoding : uint8_t
{
	UTF8,
	EUC_KR,
	Unknown
};

bool checkBOM(std::span<std::byte> data)
{
	// UTF-8 BOM 체크 (0xEF 0xBB 0xBF)
	return data.size() >= 3
		&& data[0] == std::byte(0xEF)
		&& data[1] == std::byte(0xBB)
		&& data[2] == std::byte(0xBF)
		;
}

// 인코딩 감지 (최적화: 한 번의 순회로 모든 검사 수행)
Encoding detectEncoding(std::span<std::byte> data)
{
	if (data.empty())
		return Encoding::Unknown;

	if (checkBOM(data))
		return Encoding::UTF8;

	// 한 번의 순회로 UTF-8 유효성, 한글 포함 여부, EUC-KR 패턴을 모두 검사
	bool isValidUtf8 = true;
	bool hasUtf8Korean = false;
	bool hasEucKrPattern = false;

	size_t i = 0;
	while (i < data.size())
	{
		unsigned char byte = static_cast<unsigned char>(data[i]);

		// ASCII (0x00-0x7F)
		if (byte <= 0x7F)
		{
			i++;
			continue;
		}

		// UTF-8 멀티바이트 검사
		if (isValidUtf8)
		{
			// 2바이트 UTF-8 (110xxxxx 10xxxxxx)
			if ((byte & 0xE0) == 0xC0)
			{
				if (i + 1 >= data.size() || (static_cast<unsigned char>(data[i + 1]) & 0xC0) != 0x80)
				{
					isValidUtf8 = false;
				}
				else
				{
					i += 2;
					continue;
				}
			}
			// 3바이트 UTF-8 (1110xxxx 10xxxxxx 10xxxxxx) - 한글 범위 포함
			else if ((byte & 0xF0) == 0xE0)
			{
				if (i + 2 >= data.size()
					|| (static_cast<unsigned char>(data[i + 1]) & 0xC0) != 0x80
					|| (static_cast<unsigned char>(data[i + 2]) & 0xC0) != 0x80)
				{
					isValidUtf8 = false;
				}
				else
				{
					// UTF-8 한글 범위 체크 (0xEA-0xED)
					if (byte >= 0xEA && byte <= 0xED)
						hasUtf8Korean = true;

					i += 3;
					continue;
				}
			}
			// 4바이트 UTF-8 (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
			else if ((byte & 0xF8) == 0xF0)
			{
				if (i + 3 >= data.size()
					|| (static_cast<unsigned char>(data[i + 1]) & 0xC0) != 0x80
					|| (static_cast<unsigned char>(data[i + 2]) & 0xC0) != 0x80
					|| (static_cast<unsigned char>(data[i + 3]) & 0xC0) != 0x80)
				{
					isValidUtf8 = false;
				}
				else
				{
					i += 4;
					continue;
				}
			}
			else
			{
				isValidUtf8 = false;
			}
		}

		// EUC-KR 패턴 검사 (UTF-8이 유효하지 않을 때만)
		if (isValidUtf8 == false && hasEucKrPattern == false && i + 1 < data.size())
		{
			unsigned char curr = static_cast<unsigned char>(data[i]);
			unsigned char next = static_cast<unsigned char>(data[i + 1]);

			// EUC-KR 한글 범위 (0xB0-0xC8, 0xA1-0xFE)
			if ((curr >= 0xB0 && curr <= 0xC8) && (next >= 0xA1 && next <= 0xFE))
				hasEucKrPattern = true;
		}

		i++;
	}

	// UTF-8이 유효하면 UTF-8 반환
	if (isValidUtf8)
		return Encoding::UTF8;

	// EUC-KR 패턴이 발견되면 EUC-KR 반환
	if (hasEucKrPattern)
		return Encoding::EUC_KR;

	return Encoding::Unknown;
}

// EUC-KR to UTF-8 변환 (최적화: offset 기반)
// 주의: 이 함수는 기본적인 변환만 수행합니다. 완전한 EUC-KR 지원을 위해서는
// iconv 또는 ICU 라이브러리 사용을 권장합니다.
std::u8string convertEucKrToUtf8(std::span<std::byte> data)
{
	if (data.empty())
		return std::u8string{};

	std::u8string result;
	result.reserve(data.size() * 2); // 최대 2배 크기로 예약

	size_t i = 0;
	while (i < data.size())
	{
		unsigned char curr = static_cast<unsigned char>(data[i]);

		// ASCII 범위
		if (curr < 0x80)
		{
			result.push_back(static_cast<char8_t>(curr));
			i++;
			continue;
		}

		// EUC-KR 2바이트 문자
		if (i + 1 < data.size())
		{
			unsigned char next = static_cast<unsigned char>(data[i + 1]);

			// 한글 범위 체크 (0xB0-0xC8, 0xA1-0xFE)
			if ((curr >= 0xB0 && curr <= 0xC8) && (next >= 0xA1 && next <= 0xFE))
			{
				// EUC-KR 코드를 유니코드 코드 포인트로 변환
				unsigned int code = ((curr - 0xB0) * 94) + (next - 0xA1) + 0xAC00;

				// UTF-8로 인코딩 (한글 범위: U+AC00 ~ U+D7A3)
				if (code >= 0xAC00 && code <= 0xD7A3)
				{
					// 3바이트 UTF-8 인코딩
					result.push_back(static_cast<char8_t>(0xE0 | ((code >> 12) & 0x0F)));
					result.push_back(static_cast<char8_t>(0x80 | ((code >> 6) & 0x3F)));
					result.push_back(static_cast<char8_t>(0x80 | (code & 0x3F)));
					i += 2;
					continue;
				}
			}
		}

		// 변환할 수 없는 경우 '?'로 대체
		result.push_back(u8'?');
		i++;
	}

	return result;
}
