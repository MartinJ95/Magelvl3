project(Graphics)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/Resources/include
	${additional_includes}
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
	"${PROJECT_SOURCE_DIR}/Resources/include/imgui-1.91.8/backends/imgui_impl_glfw.cpp"
	"${PROJECT_SOURCE_DIR}/Resources/include/imgui-1.91.8/backends/imgui_impl_vulkan.cpp"
	"${PROJECT_SOURCE_DIR}/Resources/include/imgui-1.91.8/backends/imgui_impl_win32.cpp"
	"${PROJECT_SOURCE_DIR}/Resources/include/imgui-1.91.8/imgui_tables.cpp"
	"${PROJECT_SOURCE_DIR}/Resources/include/imgui-1.91.8/imgui_widgets.cpp"
	"${PROJECT_SOURCE_DIR}/Resources/include/imgui-1.91.8/imgui_draw.cpp"
	"${PROJECT_SOURCE_DIR}/Resources/include/imgui-1.91.8/imgui_demo.cpp"
	"${PROJECT_SOURCE_DIR}/Resources/include/imgui-1.91.8/imgui_demo.cpp"
	"${PROJECT_SOURCE_DIR}/Resources/include/imgui-1.91.8/imgui.cpp"
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

if("${graphics_to_use}" STREQUAL "vulkan")

	find_package(Vulkan REQUIRED)
	add_subdirectory(Resources/Vulkan-Hpp/glm EXCLUDE_FROM_ALL)

	add_library(Graphics ${all_SRCS})
	target_link_libraries(Graphics glfw3 glslangd glslang-default-resource-limitsd SPIRVd MachineIndependentd OGLCompilerd OSDependentd GenericCodeGend Vulkan::Vulkan glm utils Utilities)

elseif("${graphics_to_use}" STREQUAL "none")

	add_library(Graphics ${all_SRCS})
	target_link_libraries(Utilities)

endif()