#include <iostream>
#include <document/document.h>

#include <format>
#include <filesystem>

namespace fs = std::filesystem;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

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
		std::wcout << L"[start]";
		print_u8(document->getLine(l));
		
		std::wcout << L"[end]" << std::endl;
	}
	std::wcout << L"[--------------------------------------------]" << std::endl;

	return 0;
}
