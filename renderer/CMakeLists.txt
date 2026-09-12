cmake_minimum_required(VERSION 3.16)

project(myrenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------
# mygpu dependency
# --------------------------------------------------

add_library(mygpu STATIC IMPORTED)

set_target_properties(mygpu PROPERTIES
    IMPORTED_LOCATION
        "${CMAKE_CURRENT_SOURCE_DIR}/../build/libmygpu.a"
    INTERFACE_INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

# --------------------------------------------------
# C++ renderer library
# --------------------------------------------------

add_library(myrenderer STATIC
    src/renderer.cpp 
    src/buffer.cpp 
    src/commandBuffer.cpp
)

target_include_directories(myrenderer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(myrenderer PUBLIC mygpu)

target_compile_options(myrenderer
    PRIVATE 
        -Wall
        -Wextra 
        -Wpedantic 
)

# --------------------------------------------------
# Tests
# --------------------------------------------------

add_executable(test_renderer
    tests/testRenderer.cpp 
)

target_link_libraries(test_renderer PRIVATE myrenderer)

target_compile_options(test_renderer
    PRIVATE 
        -Wall
        -Wextra
        -Wpedantic
)

enable_testing()

add_test(
    NAME renderer
    COMMAND test_renderer
)