cmake_minimum_required(VERSION 3.10)
project(simage)

set(CMAKE_CXX_STANDARD 17)

add_executable(simagec client.cpp crypt.cpp crypt.h utils.cpp utils.h)

add_executable(simages server.cpp crypt.cpp crypt.h utils.cpp utils.h)

TARGET_LINK_LIBRARIES(simagec pthread sodium)
TARGET_LINK_LIBRARIES(simages pthread sodium)