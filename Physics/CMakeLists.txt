project(Physics)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.c"
        )

add_library(Physics ${all_SRCS})
target_link_libraries(Physics Utilities)
target_include_directories(Physics PUBLIC include src)