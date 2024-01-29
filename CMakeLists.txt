cmake_minimum_required(VERSION 3.5)
project(toyuv)

add_executable(toyuv src/toyuv.cpp)

target_compile_options(toyuv PRIVATE -std=c++20)
target_link_options(toyuv PRIVATE -static)

add_custom_target(run
    COMMAND toyuv ${CMAKE_CURRENT_LIST_DIR}/image/image.ppm
    DEPENDS toyuv ${CMAKE_CURRENT_LIST_DIR}/image/image.ppm
)