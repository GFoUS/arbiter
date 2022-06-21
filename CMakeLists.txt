cmake_minimum_required(VERSION 3.8)

project(arbiter)

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