cmake_minimum_required(VERSION 3.25)
project(SimpleHttpServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-15)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fsanitize=thread")
add_executable(SimpleHttpServer main.cpp HttpServer.hpp Request.hpp ThreadPool.hpp WorkingThread.hpp NonBlockingQueue.hpp)

