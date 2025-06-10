cmake_minimum_required(VERSION 3.10)
project(ServerUDPFTP)
add_executable(ServerUDPFTP server.cpp socket_interface.cpp crc32.cpp)

project(ClientUDPFTP)
add_executable(ClientUDPFTP client.cpp socket_interface.cpp crc32.cpp)
