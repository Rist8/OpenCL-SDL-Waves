cmake_minimum_required(VERSION 3.7)

project(SDL)

find_package(SDL2 REQUIRED)
find_package(OpenCL REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_compile_options(-O3)
add_executable(SDL Source.cpp)
target_link_libraries(SDL ${SDL2_LIBRARIES} OpenCL)

