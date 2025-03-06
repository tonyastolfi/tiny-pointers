cmake_minimum_required(VERSION 3.20)

cxx_library(tiny_pointers)

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -march=native -mbmi2 -mavx2 -msse2 -msse4.2")
#set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer")

target_link_libraries(
  tiny_pointers
  PUBLIC boost::boost
  PUBLIC batteries::batteries
  PUBLIC bitmagic::bitmagic
  PUBLIC xxHash::xxhash
  #
  # Add other library -> library dependencies here.
  #
  )

target_link_libraries(
  tiny_pointers_Test
  PRIVATE gtest::gtest
  #
  # Add other test -> library dependencies here
  #
  )
