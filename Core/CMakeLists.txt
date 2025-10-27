project(Core)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
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
target_include_directories(Core INTERFACE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)