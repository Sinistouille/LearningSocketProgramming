cmake_minimum_required(VERSION 3.29)
project(Network)

set(CMAKE_CXX_STANDARD 20)

link_libraries(ws2_32 ws2_32)

add_executable(server src/server.c
        src/packet/packet.h)
add_executable(client src/client.c
        src/packet/packet.h)

link_directories(src/http/server/)
add_executable(serverhttp src/http/main.c src/packet/packet.h src/http/server/httpserver.h
         src/http/server/httpServer.c)