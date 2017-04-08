cmake_minimum_required(VERSION 2.8.8)

project(crlfstat)

if(UNIX)
	add_definitions(-std=c++11)
endif(UNIX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(${PROJECT_NAME} main.cpp)