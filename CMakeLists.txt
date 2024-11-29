cmake_minimum_required(VERSION 3.29)
project(Network)

set(CMAKE_CXX_STANDARD 20)

link_libraries(ws2_32 ws2_32)

add_executable(server server.c
        src/packet.h)
add_executable(client client.c
        src/packet.h)

