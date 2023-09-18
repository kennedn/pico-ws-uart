cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

include(pico_sdk_import.cmake)

project(pico_ws_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_subdirectory(lib)
add_subdirectory(app)
