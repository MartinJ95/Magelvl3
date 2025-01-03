project(Graphics)

set(graphics_to_use "vulkan")

set(all_includes 
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/Resources/include
)
if("${graphics_to_use}" STREQUAL "vulkan")
list(APPEND all_includes
	${PROJECT_SOURCE_DIR}/include/vulkan
        ${PROJECT_SOURCE_DIR}/src/vulkan
	${PROJECT_SOURCE_DIR}/Resources/Vulkan-Hpp
	${PROJECT_SOURCE_DIR}/Resources/Vulkan-Hpp/Vulkan-Headers/include
)
elseif("${graphics_to_use}" STREQUAL "none")
list(APPEND all_includes
	${PROJECT_SOURCE_DIR}/include/none
        ${PROJECT_SOURCE_DIR}/src/none
)
endif()

include_directories(
        ${all_includes}
)

link_directories(
	${PROJECT_SOURCE_DIR}/Resources/lib
)

set(all_files 
	"${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.c"
)

if("${graphics_to_use}" STREQUAL "vulkan")
list(APPEND all_files
	"${PROJECT_SOURCE_DIR}/include/vulkan/*.h"
        "${PROJECT_SOURCE_DIR}/include/vulkan/*.hpp"
	"${PROJECT_SOURCE_DIR}/src/vulkan/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/vulkan/*.c"
)
elseif("${graphics_to_use}" STREQUAL "none")
list(APPEND all_files
	"${PROJECT_SOURCE_DIR}/include/none/*.h"
        "${PROJECT_SOURCE_DIR}/include/none/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/none/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/none/*.c"
)
endif()

file(GLOB all_SRCS
        ${all_files}
        )

find_package(Vulkan REQUIRED)
add_subdirectory(Resources/Vulkan-Hpp/glm EXCLUDE_FROM_ALL)

add_library(Graphics ${all_SRCS})
target_link_libraries(Graphics glfw3 glslangd glslang-default-resource-limitsd SPIRVd MachineIndependentd OGLCompilerd OSDependentd GenericCodeGend Vulkan::Vulkan glm utils)