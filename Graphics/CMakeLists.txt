project(Graphics)

set(GLOB graphics_to_use "vulkan")

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/Resources/include
	if(&{graphics_to_use} == "vulkan")
{
	${PROJECT_SOURCE_DIR}/include/vulkan
        ${PROJECT_SOURCE_DIR}/src/vulkan
	${PROJECT_SOURCE_DIR}/Resources/Vulkan-Hpp
	${PROJECT_SOURCE_DIR}/Resources/Vulkan-Hpp/Vulkan-Headers/include
}
)

link_directories(
	${PROJECT_SOURCE_DIR}/Resources/lib
)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.c"
if(&{graphics_to_use} == "vulkan")
{
	"${PROJECT_SOURCE_DIR}/include/vulkan/*.h"
        "${PROJECT_SOURCE_DIR}/include/vulkan/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/vulkan/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/vulkan/*.c"
}
        )

find_package(Vulkan REQUIRED)
add_subdirectory(Resources/Vulkan-Hpp/glm EXCLUDE_FROM_ALL)

add_library(Graphics ${all_SRCS})
target_link_libraries(Graphics glfw3 glslangd glslang-default-resource-limitsd SPIRVd MachineIndependentd OGLCompilerd OSDependentd GenericCodeGend Vulkan::Vulkan glm utils)