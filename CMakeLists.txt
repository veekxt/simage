cmake_minimum_required(VERSION 3.10)
project(simage)

set(CMAKE_CXX_STANDARD 17)

add_executable(simagec client.cpp )

add_executable(simages server.cpp)

TARGET_LINK_LIBRARIES(simagec pthread)