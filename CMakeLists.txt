cmake_minimum_required(VERSION 4.0)

project(HIGHPERFORMANCESERVER)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_BUILD_TYPE Debug)

include_directories(include)

add_executable(server ./src/main.cpp ./src/server.cpp ./src/channel.cpp ./src/epoll.cpp ./src/eventloop.cpp)