# macros.cmake
# CMake 4.0.0+ compatible

# Set C++ language standard for the project
# Call this after project() command
macro (set_language_standard)
    message(STATUS "Setting C++20 standard")
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_C_STANDARD 17)
    set(CMAKE_C_STANDARD_REQUIRED ON)
endmacro ()

# Set Windows subsystem for a target
function (subsystem target sub)
    set_target_properties(${target} PROPERTIES LINK_FLAGS "/SUBSYSTEM:${sub}")
    if (sub STREQUAL "WINDOWS")
        target_compile_definitions(${target} PRIVATE _WINDOWS)
    elseif (sub STREQUAL "CONSOLE")
        target_compile_definitions(${target} PRIVATE _CONSOLE)
    endif ()
endfunction ()

# Set VS debugger working directory
function (set_working_dir target)
    set_target_properties(${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
endfunction ()

# Create executable with subsystem setting
function (make_executable name system)
    add_executable(${name})
    subsystem(${name} ${system})
    set_working_dir(${name})
endfunction ()

# Create library (STATIC, SHARED, MODULE, OBJECT, INTERFACE)
function (make_library name type)
    string(TOUPPER "${type}" type_upper)
    add_library(${name} ${type_upper})

    if (type_upper STREQUAL "SHARED")
        subsystem(${name} "WINDOWS")
    endif ()
    set_working_dir(${name})
endfunction ()

# Set precompiled header using CMake native support (3.16+)
function (set_pch target src header)
    target_precompile_headers(${target} PRIVATE ${header})
endfunction ()

# Skip precompiled header for specific source files
function (unset_pch target)
    set_source_files_properties(${ARGN}
        TARGET_DIRECTORY ${target}
        PROPERTIES SKIP_PRECOMPILE_HEADERS ON
    )
endfunction ()

# Add source files to target with source group
function (add_sources target group)
    set(sources ${ARGN})
    source_group(${group} FILES ${sources})
    target_sources(${target} PRIVATE ${sources})
endfunction ()

# Add source files without precompiled header
function (add_sources_no_pch target group)
    add_sources(${target} ${group} ${ARGN})
    unset_pch(${target} ${ARGN})
endfunction ()

# Add precompiled header source and header to target
function (add_pch target src header)
    set(sources
        ${src}
        ${header}
    )
    add_sources(${target} "pch" ${sources})
    set_pch(${target} ${src} ${header})
endfunction ()

# Add dependency and link library
function (add_and_link_dependency target dependency)
    add_dependencies(${target} ${dependency})

    target_link_libraries(${target}
        PUBLIC
            ${dependency}
    )
endfunction ()
