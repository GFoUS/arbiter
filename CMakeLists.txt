cmake_minimum_required(VERSION 3.8)

project(arbiter)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

add_subdirectory(../aether aether)

file(GLOB SRC
    "src/**/**/*.c"
    "src/**/*.c"
    "src/*.c"
)

include_directories(src)

add_executable(arbiter 
    ${SRC}
)
target_link_libraries(arbiter aether)