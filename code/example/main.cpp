#include <iostream>
#include <document/document.h>

#include <format>
#include <filesystem>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <WinBase.h>
#include <processenv.h>
#include <stringapiset.h>
#include <WinNls.h>

namespace
{
	void write_colored(wchar_t ch, WORD attributes)
	{
		HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
		if (out == nullptr || out == INVALID_HANDLE_VALUE)
		{
			std::wcout << ch;
			return;
		}

		CONSOLE_SCREEN_BUFFER_INFO csbi{};
		if (GetConsoleScreenBufferInfo(out, &csbi) == 0)
		{
			std::wcout << ch;
			return;
		}

		std::wcout.flush();

		const WORD original = csbi.wAttributes;
		SetConsoleTextAttribute(out, attributes);
		DWORD written = 0;
		WriteConsoleW(out, &ch, 1, &written, nullptr);
		SetConsoleTextAttribute(out, original);
	}

	WORD current_background()
	{
		HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
		if (out == nullptr || out == INVALID_HANDLE_VALUE)
			return 0;

		CONSOLE_SCREEN_BUFFER_INFO csbi{};
		if (GetConsoleScreenBufferInfo(out, &csbi) == 0)
			return 0;

		return csbi.wAttributes & 0xF0;
	}

	void write_start_marker()
	{
		write_colored(L'[', current_background() | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	}

	void write_end_marker()
	{
		write_colored(L']', current_background() | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	}
}

void print_u8(std::u8string_view u8view)
{
	// UTF-8 → UTF-16 길이 계산
	int len = MultiByteToWideChar(CP_UTF8
		, 0
		, reinterpret_cast<const char*>(u8view.data())
		, static_cast<int>(u8view.size())
		, nullptr
		, 0
	);

	std::vector<TCHAR> buffer;
	buffer.resize(len + 1, L'\0');

	MultiByteToWideChar(CP_UTF8
		, 0
		, reinterpret_cast<const char*>(u8view.data())
		, static_cast<int>(u8view.size())
		, buffer.data()
		, len
	);

	DWORD written;
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE)
		, buffer.data()
		, static_cast<DWORD>(buffer.size())
		, &written
		, nullptr
	);
}

int main()
{
	std::wcout.imbue(std::locale{ ".949" });
	std::wcerr.imbue(std::locale{ ".949" });


	fs::path path = ROOT_DIR;
	//	auto document = yul::TextDocument::make(path / "LiCENSE.md");
	auto document = yul::TextDocument::make(path / "code/Document/document.cpp");
	//auto document = yul::TextDocument::make(path / "test.txt");
	if (document == nullptr)
	{
		std::wcerr << L"Failed to create TextDocument from file: " << path << std::endl;
		return 1;
	}


	std::wcout << L"[--------------------------------------------]" << std::endl;
	for (yul::line_type l = 0; l < document->lineLength(); ++l)
	{
		write_start_marker();
		print_u8(document->getLine(l));

		write_end_marker();
		std::wcout << std::endl;
	}
	std::wcout << L"[--------------------------------------------]" << std::endl;

	return 0;
}
