cmake_minimum_required(VERSION 2.8.8)

project(crlfstat)

if(UNIX)
	add_definitions(-std=c++11)
endif(UNIX)

add_subdirectory(filesystem)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} filesystem)