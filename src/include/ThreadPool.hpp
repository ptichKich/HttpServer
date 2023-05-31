#pragma once

#include "stdHeaders.hpp"
#include "WorkingThread.hpp"

class ThreadPool {
public:
    ThreadPool(size_t numThreads) {
        workingThreads.reserve(numThreads);
        for (int i=0; i<10; ++i) {
            workingThreads.emplace_back();
        }
    }

    ~ThreadPool() {

    }

    void launch() {
        for (auto& thread: workingThreads) {
            thread.start();
        }
    }

    void stop() {
        for (auto& thread: workingThreads) {
            thread.stop();
        }
    }

    void addConnection(int fd) {
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
private:
    std::vector<WorkingThread> workingThreads;

};