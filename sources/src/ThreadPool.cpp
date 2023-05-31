#include "../include/stdHeaders.hpp"
#include "../include/ThreadPool.hpp"

ThreadPool::ThreadPool(size_t numThreads) {
    workingThreads.reserve(numThreads);
    for (int i=0; i<10; ++i) {
        workingThreads.emplace_back();
    }
}

void ThreadPool::launch() {
    assert(!launched);
    for (auto& thread: workingThreads) {
        thread.start();
    }
    launched = true;
}

void ThreadPool::stop() {
    assert(launched);
    for (auto& thread: workingThreads) {
        thread.stop();
    }
}

void ThreadPool::addConnection(int fd) {
    auto iter = std::find_if(workingThreads.begin(), workingThreads.end(), [](const WorkingThread& wThread){
        return wThread.getConnectionsCount() < WorkingThread::maxConnectionsCount;
    });

    if (iter != workingThreads.end()) {
        iter->addConnection(fd);
        return;
    }

    workingThreads.resize(workingThreads.size()+1);
    workingThreads.back().addConnection(fd);
}