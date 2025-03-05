cmake_minimum_required(VERSION 3.20)
project(tiny_pointers CXX)

include(${COR_CMAKE_INCLUDE_DIR}/common.cmake)

find_package(Boost)
find_package(GTest)

add_subdirectory(src)
