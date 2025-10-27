cmake_minimum_required(VERSION 4.0)

project(Mage)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C++_STANDARD_REQUIRED True)

set(graphics_to_use CACHE STRING "vulkan")

include(FetchContent)

FetchContent_Declare(
	tinyObj
	SOURCE_DIR	${FETCHCONTENT_BASE_DIR}/tinyobj-src
	GIT_REPOSITORY	https://github.com/tinyobjloader/tinyobjloader.git
	GIT_TAG	release
)

FetchContent_Declare(
	ImGUI
	SOURCE_DIR	${FETCHCONTENT_BASE_DIR}/imgui-src
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG	v1.92.4-docking
)

FetchContent_Declare(
	Vulkan
	SOURCE_DIR	${FETCHCONTENT_BASE_DIR}/vulkan-scr
	GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Hpp.git
	GIT_TAG	v1.4.329
)

FetchContent_Declare(
	stb
	SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/stb-src
	GIT_REPOSITORY	https://github.com/nothings/stb.git
)

FetchContent_MakeAvailable(
	tinyObj ImGUI Vulkan stb
)

if("${graphics_to_use}" STREQUAL "vulkan")
set(additional_includes
	${PROJECT_SOURCE_DIR}/Graphics/include
	${PROJECT_SOURCE_DIR}/Physics/include
	${PROJECT_SOURCE_DIR}/Physics/src
	${PROJECT_SOURCE_DIR}/Utilities/include
	${PROJECT_SOURCE_DIR}/Graphics/Resources/include
	${PROJECT_SOURCE_DIR}/Graphics/include/vulkan
        ${PROJECT_SOURCE_DIR}/Graphics/src/vulkan
)
elseif("${graphics_to_use}" STREQUAL "none")
set(additional_includes
	${PROJECT_SOURCE_DIR}/Graphics/include
	${PROJECT_SOURCE_DIR}/Graphics/include/none
        ${PROJECT_SOURCE_DIR}/Graphics/src/none
	${PROJECT_SOURCE_DIR}/Physics/include
	${PROJECT_SOURCE_DIR}/Physics/src
)
endif()

add_subdirectory(Utilities)
add_subdirectory(Graphics)
add_subdirectory(Physics)
add_subdirectory(Network)
add_subdirectory(Core)
add_subdirectory(Sandbox)