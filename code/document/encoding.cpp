#include "encoding.h"

#include <cstring>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stringapiset.h>
#include <WinNls.h>

// 인코딩 감지 (최적화: 한 번의 순회로 모든 검사 수행)
yul::Encoding yul::detectEncoding(std::span<std::byte> data)
{
	if (data.empty())
		return Encoding::unknown;

	if (checkBOM(data))
		return Encoding::utf8;

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

		// EUC-KR/CP949 패턴 검사 (UTF-8이 유효하지 않을 때만)
		if (isValidUtf8 == false && hasEucKrPattern == false && i + 1 < data.size())
		{
			unsigned char curr = static_cast<unsigned char>(data[i]);
			unsigned char next = static_cast<unsigned char>(data[i + 1]);

			// EUC-KR(ksx1001) 한글 범위 (0xB0-0xC8, 0xA1-0xFE)
			bool isEucKrHangul = (curr >= 0xB0 && curr <= 0xC8) && (next >= 0xA1 && next <= 0xFE);

			// CP949 확장 범위(Windows): (0x81-0xFE, 0x41-0xFE) 단, 0x7F 제외
			// UTF-8이 깨졌을 때 "한글이 없는" CP949 텍스트도 euckr로 분류되도록 보강합니다.
			bool isCp949Dbcs = (curr >= 0x81 && curr <= 0xFE) && (next >= 0x41 && next <= 0xFE) && next != 0x7F;

			if (isEucKrHangul || isCp949Dbcs)
				hasEucKrPattern = true;
		}

		i++;
	}

	// UTF-8이 유효하면 UTF-8 반환
	if (isValidUtf8)
		return Encoding::utf8;

	// EUC-KR 패턴이 발견되면 EUC-KR 반환
	if (hasEucKrPattern)
		return Encoding::euckr;

	return Encoding::unknown;
}

// EUC-KR/CP949 → UTF-8 변환
std::u8string yul::convertEucKrToUtf8(std::span<std::byte> data)
{
	if (data.empty())
		return std::u8string{};

	const char* bytes = reinterpret_cast<const char*>(data.data());
	int bytesLen = static_cast<int>(data.size());

	auto tryConvert = [&](UINT codePage, DWORD mbFlags) -> std::u8string
	{
		int wideLen = MultiByteToWideChar(codePage, mbFlags, bytes, bytesLen, nullptr, 0);
		if (wideLen <= 0)
			return {};

		std::wstring wide;
		wide.resize(static_cast<size_t>(wideLen));
		if (MultiByteToWideChar(codePage, mbFlags, bytes, bytesLen, wide.data(), wideLen) <= 0)
			return {};

		int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLen, nullptr, 0, nullptr, nullptr);
		if (utf8Len <= 0)
			return {};

		std::string utf8;
		utf8.resize(static_cast<size_t>(utf8Len));
		if (WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLen, utf8.data(), utf8Len, nullptr, nullptr) <= 0)
			return {};

		std::u8string out;
		out.resize(utf8.size());
		std::memcpy(out.data(), utf8.data(), utf8.size());
		return out;
	};

	// 가능한 경우 EUC-KR(51949)을 우선 시도하고, 실패하면 Windows 기본(949)으로 폴백합니다.
	// 1) 엄격 모드(MB_ERR_INVALID_CHARS)로 시도
	for (UINT cp : { 51949u, 949u })
	{
		if (auto out = tryConvert(cp, MB_ERR_INVALID_CHARS); out.empty() == false)
			return out;
	}

	// 2) 치환 허용 모드로 시도 (깨진 바이트는 시스템 기본 치환 문자로 대체될 수 있음)
	for (UINT cp : { 51949u, 949u })
	{
		if (auto out = tryConvert(cp, 0); out.empty() == false)
			return out;
	}

	// 변환 불가: ASCII만 보존하고 나머지는 '?'로 대체
	std::u8string fallback;
	fallback.reserve(data.size());

	for (std::byte b : data)
	{
		auto c = static_cast<char8_t>(b);
		fallback.push_back(c < 0x80 ? c : u8'?');
	}

	return fallback;
}
