cmake_minimum_required(VERSION 3.8)

project(arbiter)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

add_subdirectory(../aether aether)

file(GLOB SRC
    "src/**/**/*.c"
    "src/**/*.c"
    "src/*.c"
)

file(GLOB IMGUIZMO_SRC
    "vendor/cimguizmo/ImGuizmo/*.cpp"
)

include_directories(
    src
    "vendor/cimguizmo"
)

add_executable(arbiter 
    ${SRC}
    ${IMGUIZMO_SRC}
    "vendor/cimguizmo/cimguizmo.cpp"
)
target_link_libraries(arbiter aether)