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

enable_testing()

set(RENDERER_TESTS
    testFrame
    testBuffer
    testCommandBuffer
    testDraw
    testCopy
)

foreach(test_name IN LISTS RENDERER_TESTS)
    add_executable(${test_name} tests/${test_name}.cpp)

    target_link_libraries(${test_name} PRIVATE myrenderer)

    target_compile_options(${test_name}
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()