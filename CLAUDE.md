# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A C++20 project that implements a TextDocument class inspired by Visual Studio Code's TextDocument API. The project handles text file loading with automatic encoding detection (UTF-8, EUC-KR) and provides position/range-based text manipulation utilities.

## IMPORTANT: File Encoding

**ALL source files (.cpp, .h) in this project are encoded in EUC-KR (CP949), NOT UTF-8.**

- Do NOT convert source files to UTF-8 encoding
- Korean comments in source files are written in EUC-KR
- The MSVC compiler expects EUC-KR encoding (code page 949)
- Converting to UTF-8 will cause C4819 compilation errors
- If you need to modify source files, ensure the encoding remains EUC-KR
- Only CLAUDE.md and other documentation files should be in UTF-8

## Build System

**Requirements:**
- Windows 11
- Visual Studio 2022+ (recommended: Visual Studio 2026)
- CMake 4.0.0+
- C++20 standard

**Build Commands:**
```bash
# From repository root
mkdir build
cd build
cmake -G "Visual Studio 18 2026 Win64" ..

# Build from Visual Studio or:
cmake --build . --config Debug
cmake --build . --config Release
```

**Build Configurations:**
- Debug
- Profile
- Release

**Output Location:**
Build artifacts are placed in `bin/$(configuration)` directory at the repository root. Set this as the working directory when debugging.

## Architecture

### Core Components

**TextDocument (code/document/document.h|cpp)**
- Factory pattern for creating document instances
- Two creation methods:
  - `make(path)`: Load from filesystem
  - `make(binary)`: Load from memory buffer
- Automatic encoding detection and conversion to UTF-8
- Internal storage uses `std::u8string` (UTF-8)

**Encoding System (code/document/encoding.h|cpp)**
- Single-pass encoding detection algorithm
- Supported encodings: UTF-8 (with/without BOM), EUC-KR, Unknown
- `detectEncoding()`: Analyzes byte patterns to determine encoding
- `convertEucKrToUtf8()`: Simplified EUC-KR to UTF-8 converter (note: basic implementation, production code should use iconv/ICU)
- `checkBOM()`: UTF-8 BOM detection

**Position & Range (code/document/position.h|range.h)**
- `Position`: Line/character coordinates with spaceship operator
- `Range`: Half-open interval [begin, end) with built-in normalization
- Range operations: `in()`, `intersect()`
- Uses `line_type` and `character_type` (both `size_t`) from type.h

### Project Structure

```
code/
├── document/          # Static library - core document functionality
│   ├── document.*     # TextDocument class
│   ├── encoding.*     # Encoding detection/conversion
│   ├── position.h     # Line/character position
│   ├── range.h        # Text range with operations
│   └── type.h         # Type aliases (line_type, character_type)
└── example/           # Console executable - example usage
    └── main.cpp       # Entry point
```

### CMake Macros (cmake/macros.cmake)

Custom macros for consistent target configuration:
- `make_executable(name system)`: Create executable with subsystem (CONSOLE/WINDOWS)
- `make_library(name type)`: Create library (STATIC/SHARED/etc)
- `add_sources(target group files...)`: Add sources with source grouping
- `add_pch(target src header)`: Configure precompiled headers
- `add_and_link_dependency(target dep)`: Add dependency and link in one call
- `set_working_dir(target)`: Set VS debugger working directory to $(OutDir)

### Platform Configuration

The project uses platform-specific configurations in `cmake/platform/windows.cmake`:
- C++20/C17 standard enforcement
- 64-bit architecture (x64)
- Windows 10 SDK auto-detection via registry or Visual Studio integration
- MSVC compiler settings in `cmake/compiler/msvc.cmake`

## Key Implementation Details

**Encoding Detection Strategy:**
- Single-pass algorithm checking for BOM, UTF-8 validity, Korean patterns
- UTF-8 validation checks multi-byte sequences (2-4 bytes)
- EUC-KR detected by pattern matching (0xB0-0xC8, 0xA1-0xFE ranges)
- Falls back to UTF-8 on unknown encoding

**TextDocument Loading:**
1. Read file as binary buffer
2. Detect encoding
3. Handle UTF-8 BOM if present (skip 3 bytes)
4. Convert EUC-KR to UTF-8 if needed
5. Store as `std::u8string`
6. Call `build()` method (currently unimplemented stub)

**Range Semantics:**
- Half-open intervals: [begin, end)
- Automatically normalizes begin/end in constructor
- Spaceship operator returns `std::partial_ordering` for overlap detection
- `intersect()` checks if ranges overlap using `in()` operations
