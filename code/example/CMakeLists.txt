set(target example)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

make_executable(${target} "CONSOLE")
add_compile_definitions(ROOT_DIR="${ROOT}")

#add_pch(${target} pch.cpp pch.h)

add_sources(${target}
	"console"
	"main.cpp"
)

add_and_link_dependency(${target} document)

target_link_libraries(${target}
	PUBLIC
		${WINLIBS}
)
