project(Core)

set(cur_path ${PROJECT_SOURCE_DIR})
cmake_path(GET cur_path PARENT_PATH parent_path)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
	${parent_path}/Graphics/include/tinyobjloader-release
	${additional_includes}

)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.c"
        )

add_library(Core ${all_SRCS})
target_link_libraries(Core Graphics Physics)