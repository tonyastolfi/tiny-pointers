cmake_minimum_required(VERSION 3.20)

cxx_library(tiny_pointers)

target_link_libraries(
  tiny_pointers
  PUBLIC boost::boost
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
