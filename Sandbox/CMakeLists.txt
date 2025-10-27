project(Sandbox)

FetchContent_MakeAvailable(
	tinyObj ImGUI Vulkan stb
)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.c"
        )

add_executable(Sandbox ${all_SRCS})
target_link_libraries(Sandbox Core)
target_include_directories(Sandbox PUBLIC include src)