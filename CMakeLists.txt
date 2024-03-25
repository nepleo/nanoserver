cmake_minimum_required(VERSION 3.22)

project(server)

set(CMAKE_CXX_STANDARD 17)

add_executable(server 
                main.cpp 
                src/epoller.h 
                src/epoller.cpp
                src/server.h
                src/server.cpp 
                src/threadpool.h 
                src/timer.h 
                src/timer.cpp
)

target_link_libraries(server PRIVATE pthread)
