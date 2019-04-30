cmake_minimum_required(VERSION 3.13)
project(coverage)

find_package(MPI REQUIRED)

#include_directories(${MPI_C_INCLUDE_PATH})
include_directories(SYSTEM ${MPI_INCLUDE_PATH})


include_directories(/usr/local/bin/mpic++)

set(CMAKE_C_COMPILER /usr/local/bin/gcc-8)
set(CMAKE_CXX_COMPILER /usr/local/bin/g++-8)


set(CMAKE_CXX_STANDARD 14)
# added -fopenmp
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")

add_executable(coverage src/main.cpp)

target_link_libraries(coverage ${MPI_LIBRARIES})
