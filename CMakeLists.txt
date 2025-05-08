cmake_minimum_required(VERSION 3.28)
project(multi_thread_server)

set(CMAKE_CXX_STANDARD 17)

add_executable(multi_thread_server src/main.cpp
)
target_link_libraries(multi_thread_server ws2_32 wsock32)
