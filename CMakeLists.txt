cmake_minimum_required(VERSION 3.20)
project(ShaderTool LANGUAGES CXX)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SDL3
	GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG release-3.2.26
	GIT_SHALLOW ON
)

FetchContent_MakeAvailable(SDL3)

add_executable(ShaderTool
	src/main.cpp
)

target_link_libraries(ShaderTool
	PRIVATE SDL3::SDL3
)

target_compile_features(ShaderTool PRIVATE cxx_std_20)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	target_compile_options(ShaderTool PRIVATE -Wall -Wextra -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(ShaderTool PRIVATE /W4)
endif()
