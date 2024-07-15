cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(spprices)

# require C++20
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_EXTENSIONS OFF )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

add_executable(plecpr ioTableAnalysis.cpp)
add_executable(plecpr-mt ioTableAnalysis_turbo.cpp)

# If you'd like these accessible 
# through first element in PATH for some reason
# install(TARGETS plecpr plecpr-mt DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)