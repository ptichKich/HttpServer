#pragma once

#include "linuxHeaders.hpp"
#include "NonBlockingQueue.hpp"
//#include "Request.hpp"


class WorkingThread {
public:
    static constexpr int maxConnectionsCount = 10000;

    WorkingThread();
    WorkingThread& operator = (WorkingThread&& other);
    WorkingThread(WorkingThread&& other);

    WorkingThread(const WorkingThread &) = delete;
    WorkingThread& operator=(const WorkingThread&) = delete;

    void start();

    ~WorkingThread();

    [[nodiscard]] int getConnectionsCount() const;

    void addConnection(int fd);

    void handleSocketEvent(int fd, uint32_t events);
    void decrementConnectionsCount();
    void stop();

private:

    std::thread innerThread;

    std::vector<int> newFds;

    NonBlockingQueue<int> fdQueue;

    std::atomic<int> connectionsCount{0};
    std::atomic<bool> stopRequested{false};
    int epollFd = -1;
};