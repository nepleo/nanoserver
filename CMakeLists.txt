cmake_minimum_required(VERSION 3.22)

project(httpserver)

set(CMAKE_CXX_STANDARD 17)

add_executable(httpserver main.cpp src/epoller.h src/epoller.cpp)
