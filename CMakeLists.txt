cmake_minimum_required(VERSION 4.0)

include(cmake/macros.cmake)

include(cmake/platform/windows.cmake)

set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

set(DEFAULT_OUTPUT_DIRECTORY "${ROOT}/bin")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DEFAULT_OUTPUT_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${DEFAULT_OUTPUT_DIRECTORY}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${DEFAULT_OUTPUT_DIRECTORY}")

link_directories(${DEFAULT_OUTPUT_DIRECTORY})

set(SOLUTION_NAME "document")

project(${SOLUTION_NAME})

add_subdirectory(code)
