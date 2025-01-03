cmake_minimum_required(VERSION 3.10)

project(Mage)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C++_STANDARD_REQUIRED True)

set(graphics_to_use "vulkan")

if("${graphics_to_use}" STREQUAL "vulkan")
set(additional_includes
	${PROJECT_SOURCE_DIR}/Graphics/include
	${PROJECT_SOURCE_DIR}/Graphics/Resources/include
	${PROJECT_SOURCE_DIR}/Graphics/include/vulkan
        ${PROJECT_SOURCE_DIR}/Graphics/src/vulkan
	${PROJECT_SOURCE_DIR}/Graphics/Resources/Vulkan-Hpp
	${PROJECT_SOURCE_DIR}/Graphics/Resources/Vulkan-Hpp/Vulkan-Headers/include
)
elseif("${graphics_to_use}" STREQUAL "none")
set(additional_includes
	${PROJECT_SOURCE_DIR}/Graphics/include/none
        ${PROJECT_SOURCE_DIR}/Graphics/src/none
)
endif()

add_subdirectory(Utils)
add_subdirectory(Graphics)
add_subdirectory(Physics)
add_subdirectory(Network)
add_subdirectory(Core)
add_subdirectory(Sandbox)