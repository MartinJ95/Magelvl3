project(Graphics)

include(FetchContent)

FetchContent_MakeAvailable(
	tinyObj ImGUI Vulkan stb
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
	"${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp"
	"${imgui_SOURCE_DIR}/imgui_tables.cpp"
	"${imgui_SOURCE_DIR}/imgui_widgets.cpp"
	"${imgui_SOURCE_DIR}/imgui_draw.cpp"
	"${imgui_SOURCE_DIR}/imgui_demo.cpp"
	"${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
	"${imgui_SOURCE_DIR}/imgui.cpp"
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

	add_library(Graphics ${all_SRCS})
	target_link_libraries(Graphics glfw glslang Vulkan::Vulkan glm utils Utilities Physics)
	target_include_directories( Graphics PUBLIC include src include/Vulkan src/Vulkan ${tinyobj_SOURCE_DIR} ${imgui_SOURCE_DIR} ${vulkan_SOURCE_DIR}/Vulkan-Headers ${vulkan_SOURCE_DIR}/glfw/include ${vulkan_SOURCE_DIR}/glm/glm ${stb_SOURCE_DIR})

elseif("${graphics_to_use}" STREQUAL "none")

	add_library(Graphics ${all_SRCS})
	target_link_libraries(Graphics PUBLIC Utilities)

endif()