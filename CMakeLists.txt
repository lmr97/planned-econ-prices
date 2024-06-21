cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(spprices)

# require C++20
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_EXTENSIONS OFF )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

add_executable(plecpr ioTableAnalysis.cpp)
install(TARGETS plecpr DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})