cmake_minimum_required(VERSION 4.0)

project(HIGHPERFORMANCESERVER)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_BUILD_TYPE Debug)

include_directories(include)

add_executable(server ./src/tcp_server.cpp)
add_executable(client ./src/tcp_client.cpp)
add_executable(rwlock ./src/rwlock.cpp)
add_executable(condition_variable ./src/condition_variable.cpp)
add_executable(sem ./src/sem.cpp)
add_executable(threadpool_c ./src/threadpool.cpp ./src/main.cpp)
