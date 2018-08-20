cmake_minimum_required(VERSION 3.10)
project(simage)

set(CMAKE_CXX_STANDARD 17)

add_executable(simage main.cpp server.cpp client.cpp crypt.cpp crypt.h utils.cpp utils.h simage.h)

TARGET_LINK_LIBRARIES(simage pthread libsodium.a )

add_definitions(-Wall -g -rdynamic)