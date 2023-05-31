cmake_minimum_required(VERSION 3.25)
project(SimpleHttpServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER /usr/bin/g++-12)

set(CMAKE_CXX_FLAGS  "-pthread -std=gnu++20")

set(HTML_DIR ${CMAKE_SOURCE_DIR}/src/html)
set(CMAKE_INCLUDE_PATH "${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/src/include")
configure_file(config.h.in config.h)
add_executable(SimpleHttpServer sources/src/main.cpp sources/include/HttpServer.hpp sources/include/Request.hpp sources/include/ThreadPool.hpp sources/include/WorkingThread.hpp sources/include/NonBlockingQueue.hpp sources/include/linuxHeaders.hpp sources/include/stdHeaders.hpp config.h sources/src/WorkingThread.cpp sources/src/HttpServer.cpp sources/include/GenericServerIface.hpp sources/src/Request.cpp sources/src/ThreadPool.cpp)
