project(Sandbox)

set(cur_path ${PROJECT_SOURCE_DIR})
cmake_path(GET cur_path PARENT_PATH parent_path)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
	${parent_path}/Core/include
	${additional_includes}
)

link_directories(
	${parent_path}/Graphics/Resources/lib
)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.c"
        )

add_executable(Sandbox ${all_SRCS})
target_link_libraries(Sandbox Core)