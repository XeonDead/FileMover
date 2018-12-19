cmake_minimum_required(VERSION 3.13)
project(FileMover)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_COMPILER "clang++")

find_package(Threads REQUIRED)

include_directories(.)

add_executable(filemover
        file.cpp
        file.h
        main.cpp)

target_link_libraries(filemover
        boost_system
        boost_filesystem
        pthread)
