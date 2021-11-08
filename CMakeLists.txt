cmake_minimum_required(VERSION 3.21)

project(p-programming-1 LANGUAGES CXX)

find_package(
    Boost 1.76 REQUIRED
    program_options
)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(p-programming-1 main.cpp auxillary.hpp)
TARGET_LINK_LIBRARIES(p-programming-1 ${Boost_LIBRARIES})

add_executable(task2 task2.cpp)
