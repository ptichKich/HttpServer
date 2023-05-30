#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "NonBlockingQueue.hpp"

class WorkingThread {
public:
    static constexpr int maxConnectionsCount = 10000;

    WorkingThread(): fdQueue(1 << 13) {

    }

    WorkingThread& operator = (WorkingThread&& other) {
        if (this != &other) {
            this->newFds = std::move(other.newFds);
            this->fdQueue = std::move(other.fdQueue);
            this->innerThread = std::move(other.innerThread);
            this->epollFd = other.epollFd;

            other.epollFd = -1;
        }
        return *this;
    }

    WorkingThread(WorkingThread&& other): fdQueue(1) {
        this->newFds = std::move(other.newFds);
        this->fdQueue = std::move(other.fdQueue);
        this->innerThread = std::move(other.innerThread);
        this->epollFd = other.epollFd;

        other.epollFd = -1;
    }

    WorkingThread(const WorkingThread &) = delete;
    WorkingThread& operator=(const WorkingThread&) = delete;

    void start() {
        epollFd = epoll_create1(0);
        if (epollFd == -1) {
            std::cerr << "Error creating epoll: " << strerror(errno) << std::endl;
            exit(1);
        }

        newFds.reserve(maxConnectionsCount);
        innerThread = std::thread([this]() {
            epoll_event events[maxConnectionsCount];
            while (!stopRequested) {
                while (!fdQueue.empty()) {
                    int fd;

                    while(!fdQueue.pop(fd)){}
                    newFds.push_back(fd);
                }

                if (!newFds.empty()) {
                    for (auto fd: newFds) {
                        epoll_event event{};
                        event.events = EPOLLIN | EPOLLHUP | EPOLLOUT;
                        event.data.fd = fd;

                        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
                            // TODO: write smth reasonable std::cerr << "Error: " << strerror(errno) << std::endl;
                            close(epollFd);
                            exit(1);
                        }
                        ++connectionsCount;
                    }
                    newFds.clear();
                }

                if (connectionsCount == 0) {
                    continue;
                }

                int numEvents = epoll_wait(epollFd, events, maxConnectionsCount, -1);
                if (numEvents == -1) {
                    std::cerr << "Error epoll wait: " << strerror(errno) << std::endl;
                }

                for (int i=0; i<numEvents; ++i ) {
                    // TODO: must send response or remove from epoll
                    handleSocketEvent(events[i].data.fd, events[i].events);
                }
            }
        });
    }

    ~WorkingThread() {
        innerThread.join();
        if (epollFd != -1) {
            close(epollFd);
        }
        //TODO: implement all operations to safely close all descriptors
    }

    [[nodiscard]] int getConnectionsCount() const {
        return connectionsCount;
    }

    void addConnection(int fd) {
        while(!fdQueue.push(fd)){}
    }

    void handleSocketEvent(int fd, uint32_t events) {
        if (events & EPOLLHUP) {
            close(fd);
            return;
        }


    }

    void stop() {
        stopRequested = true;
    }

private:

    std::thread innerThread;

    std::vector<int> newFds;

    NonBlockingQueue<int> fdQueue;

    std::atomic<int> connectionsCount{0};
    std::atomic<bool> stopRequested{false};
    int epollFd = -1;
};