# CMake 4.0+ 호환성을 위한 Windows 플랫폼 설정
cmake_minimum_required(VERSION 3.21)

set_language_standard()

set(CMAKE_CONFIGURATION_TYPES Debug Profile Release)
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Reset the configurations to what we need" FORCE)

# 64bit 아키텍처 설정
set(BUILD_CPU_ARCHITECTURE "x64")
set(BUILD_PLATFORM "WIN64")
set(PLATFORM_NAME "win64")
set(WINDOWS ON)
set(WIN64 ON)

message(STATUS "BUILD_CPU_ARCHITECTURE = ${BUILD_CPU_ARCHITECTURE}")

set(CMAKE_GENERATOR_TOOLSET "host=x64")

# CMake 4.0+: add_definitions() 대신 add_compile_definitions() 사용
add_compile_definitions(_WINDOWS)
# add_compile_definitions(WIN32 _WIN32)
# add_compile_definitions(WIN64 _WIN64)

set(CMAKE_VS_PLATFORM_NAME "x64")
include(${CMAKE_SOURCE_DIR}/cmake/compiler/msvc.cmake)

set(WINLIBS "")
list(APPEND WINLIBS "winmm.lib")

# Windows 10 SDK 탐지 (CMake 4.0+ 호환)
# 우선순위:
#   1. 사용자 지정 SDK_DIR
#   2. Visual Studio 통합을 통한 자동 탐지
#   3. 레지스트리 기반 탐지
function(find_windows_10_sdk)
    # 이미 설정되어 있으면 반환
    if(DEFINED WINDOWS_SDK AND EXISTS "${WINDOWS_SDK}")
        message(STATUS "Windows SDK (사용자 지정): ${WINDOWS_SDK}")
        return()
    endif()

    # 1. 사용자 지정 SDK_DIR 확인
    if(DEFINED SDK_DIR AND EXISTS "${SDK_DIR}/Microsoft Windows SDK/10")
        set(WINDOWS_SDK "${SDK_DIR}/Microsoft Windows SDK/10" PARENT_SCOPE)
        message(STATUS "Windows SDK (SDK_DIR): ${SDK_DIR}/Microsoft Windows SDK/10")
        return()
    endif()

    # 2. CMake의 Visual Studio 통합 활용 (권장)
    # CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION은 VS Generator 사용 시 자동 설정됨
    if(DEFINED CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        # Windows Kits 경로 탐지
        cmake_host_system_information(RESULT _program_files_x86 QUERY WINDOWS_REGISTRY
            "HKLM/SOFTWARE/Microsoft/Windows Kits/Installed Roots"
            VALUE "KitsRoot10"
            ERROR_VARIABLE _reg_error)

        if(NOT _reg_error AND EXISTS "${_program_files_x86}")
            set(WINDOWS_SDK "${_program_files_x86}" PARENT_SCOPE)
            set(WINDOWS_SDK_VERSION "${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}" PARENT_SCOPE)
            message(STATUS "Windows SDK (VS 통합): ${_program_files_x86}")
            message(STATUS "Windows SDK 버전: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
            return()
        endif()
    endif()

    # 3. 레지스트리에서 직접 탐색 (CMake 3.24+ 방식)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
        cmake_host_system_information(RESULT _kits_root QUERY WINDOWS_REGISTRY
            "HKLM/SOFTWARE/Microsoft/Windows Kits/Installed Roots"
            VALUE "KitsRoot10"
            ERROR_VARIABLE _reg_error)

        if(NOT _reg_error AND EXISTS "${_kits_root}")
            set(WINDOWS_SDK "${_kits_root}" PARENT_SCOPE)

            # 설치된 SDK 버전 중 최신 버전 찾기
            file(GLOB _sdk_versions LIST_DIRECTORIES true "${_kits_root}/Include/10.*")
            if(_sdk_versions)
                list(SORT _sdk_versions COMPARE NATURAL ORDER DESCENDING)
                list(GET _sdk_versions 0 _latest_sdk)
                get_filename_component(_sdk_version "${_latest_sdk}" NAME)
                set(WINDOWS_SDK_VERSION "${_sdk_version}" PARENT_SCOPE)
                message(STATUS "Windows SDK (레지스트리): ${_kits_root}")
                message(STATUS "Windows SDK 버전 (자동 탐지): ${_sdk_version}")
            endif()
            return()
        endif()
    else()
        # CMake 3.24 미만: 기존 방식 사용
        get_filename_component(_kits_root
            "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]"
            ABSOLUTE)

        if(EXISTS "${_kits_root}")
            set(WINDOWS_SDK "${_kits_root}" PARENT_SCOPE)
            message(STATUS "Windows SDK (레거시): ${_kits_root}")
            return()
        endif()
    endif()

    message(WARNING "Windows 10 SDK를 찾을 수 없습니다. SDK_DIR을 설정하거나 Windows SDK를 설치하세요.")
endfunction()

# Windows SDK 탐지 실행
find_windows_10_sdk()

# SDK 경로 유효성 검증
if(DEFINED WINDOWS_SDK)
    if(NOT EXISTS "${WINDOWS_SDK}")
        message(WARNING "WINDOWS_SDK 경로가 존재하지 않습니다: ${WINDOWS_SDK}")
        unset(WINDOWS_SDK)
    endif()
endif()
