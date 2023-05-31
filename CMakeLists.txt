cmake_minimum_required(VERSION 3.25)
project(SimpleHttpServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER /usr/bin/g++-12)

set(CMAKE_CXX_FLAGS  "-pthread -std=gnu++20")

set(HTML_DIR ${CMAKE_SOURCE_DIR}/src/html)
set(CMAKE_INCLUDE_PATH "${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/src/include")
configure_file(config.h.in config.h)
add_executable(SimpleHttpServer src/main.cpp src/include/HttpServer.hpp src/include/Request.hpp src/include/ThreadPool.hpp src/include/WorkingThread.hpp src/include/NonBlockingQueue.hpp src/include/linuxHeaders.hpp src/include/stdHeaders.hpp config.h src/WorkingThread.cpp)

