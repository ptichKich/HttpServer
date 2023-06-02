cmake_minimum_required(VERSION 3.25)
project(SimpleHttpServer)

set(CMAKE_CXX_STANDARD 20)

function(addTargets)
    add_executable(SimpleHttpServer sources/src/main.cpp sources/include/HttpServer.hpp sources/include/Request.hpp sources/include/ThreadPool.hpp sources/include/WorkingThread.hpp sources/include/NonBlockingQueue.hpp sources/include/linuxHeaders.hpp sources/include/stdHeaders.hpp sources/src/WorkingThread.cpp sources/src/HttpServer.cpp sources/include/GenericServerIface.hpp sources/src/Request.cpp sources/src/ThreadPool.cpp )
    add_executable(Test sources/Test/test.cpp sources/include/HttpServer.hpp sources/include/Request.hpp sources/include/ThreadPool.hpp sources/include/WorkingThread.hpp sources/include/NonBlockingQueue.hpp sources/include/linuxHeaders.hpp sources/include/stdHeaders.hpp sources/src/WorkingThread.cpp sources/src/HttpServer.cpp sources/include/GenericServerIface.hpp sources/src/Request.cpp sources/src/ThreadPool.cpp )
endfunction()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS  "-pthread -g")
    set(CMAKE_INCLUDE_PATH "${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/src/include")

    addTargets()
elseIf (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS  "-pthread -O2")
    set(CMAKE_INCLUDE_PATH "${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/src/include")

    addTargets()
endif ()
