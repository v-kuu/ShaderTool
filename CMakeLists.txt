cmake_minimum_required(VERSION 3.20)
project(ShaderTool LANGUAGES CXX)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(add_slang_shader_target TARGET)
	cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})

	set(SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)

	add_custom_command(
		OUTPUT ${SHADERS_DIR}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
	)

	add_custom_command(
		OUTPUT ${SHADERS_DIR}/slang.spv
		COMMAND ${SLANG_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o slang.spv
		WORKING_DIRECTORY ${SHADERS_DIR}
		DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
		COMMENT "Compiling Slang Shaders"
		VERBATIM
	)

	add_custom_target(${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

#add_slang_shader_target(foo SOURCES ${SHADER_SLANG_SOURCES})
#add_dependencies(bar foo)

include(FetchContent)

FetchContent_Declare(SDL3
	GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG release-3.2.26
	GIT_SHALLOW ON
)

FetchContent_Declare(glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 0af55ccecd98d4e5a8d1fad7de25ba429d60e863
	GIT_SHALLOW ON
)

FetchContent_MakeAvailable(SDL3 glm)

find_package(Vulkan REQUIRED)

add_executable(ShaderTool
	src/main.cpp
	src/Application.cpp
)

target_link_libraries(ShaderTool
	PRIVATE SDL3::SDL3
	PRIVATE glm::glm
	PRIVATE Vulkan::Vulkan
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	target_compile_options(ShaderTool PRIVATE -Wall -Wextra -Werror -g)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(ShaderTool PRIVATE /W4)
endif()

message(STATUS "CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
