cmake_minimum_required(VERSION 3.20)
project(ShaderTool LANGUAGES CXX)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(SDL3
	GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG release-3.2.26
	GIT_SHALLOW ON
)

FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 0af55ccecd98d4e5a8d1fad7de25ba429d60e863
)

FetchContent_Declare(
	VulkanHeaders
	GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
	GIT_TAG v1.4.334
)

set(BUILD_STATIC_LOADER ON CACHE BOOL "Build static Vulkan loader" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
	VulkanLoader
	GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
	GIT_TAG v1.4.333
)

FetchContent_MakeAvailable(SDL3 glm VulkanHeaders VulkanLoader)

add_executable(ShaderTool
	src/main.cpp
)

target_link_libraries(ShaderTool
	PRIVATE SDL3::SDL3
	PRIVATE glm::glm
	PRIVATE Vulkan::Vulkan
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	target_compile_options(ShaderTool PRIVATE -Wall -Wextra -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(ShaderTool PRIVATE /W4)
endif()
