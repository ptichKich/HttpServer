cmake_minimum_required(VERSION 3.25)
project(SimpleHttpServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-15)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fsanitize=thread")
add_executable(SimpleHttpServer src/main.cpp src/HttpServer.hpp src/Request.hpp src/ThreadPool.hpp src/WorkingThread.hpp src/NonBlockingQueue.hpp)

