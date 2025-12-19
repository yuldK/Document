#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace yul
{
	// 인코딩 타입
	enum class Encoding : uint8_t
	{
		utf8,
		euckr,
		unknown
	};

	inline bool checkBOM(std::span<std::byte> data)
	{
		// UTF-8 BOM 체크 (0xEF 0xBB 0xBF)
		return data.size() >= 3
			&& data[0] == std::byte(0xEF)
			&& data[1] == std::byte(0xBB)
			&& data[2] == std::byte(0xBF)
			;
	}

	// 인코딩 감지 (최적화: 한 번의 순회로 모든 검사 수행)
	Encoding detectEncoding(std::span<std::byte> data);

	// EUC-KR/CP949 → UTF-8 변환
	// Windows에서는 OS 변환 API(MultiByteToWideChar/WideCharToMultiByte)를 사용합니다.
	std::u8string convertEucKrToUtf8(std::span<std::byte> data);
}
