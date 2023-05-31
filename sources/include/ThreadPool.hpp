#pragma once

#include "WorkingThread.hpp"

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);

    ~ThreadPool() = default;

    void launch();
    void stop();
    void addConnection(int fd);
private:
    std::vector<WorkingThread> workingThreads;
    bool launched = false;
};