#include "../include/linuxHeaders.hpp"
#include "../include/stdHeaders.hpp"
#include "../include/WorkingThread.hpp"
#include "../include/Request.hpp"

WorkingThread::WorkingThread(): fdQueue(1 << 13) {

}

WorkingThread& WorkingThread::operator=(WorkingThread &&other) {
    if (this != &other) {
        this->newFds = std::move(other.newFds);
        this->fdQueue = std::move(other.fdQueue);
        this->innerThread = std::move(other.innerThread);
        this->epollFd = other.epollFd;

        other.epollFd = -1;
    }
    return *this;
}

WorkingThread::WorkingThread(WorkingThread &&other) : fdQueue(1) {
    this->newFds = std::move(other.newFds);
    this->fdQueue = std::move(other.fdQueue);
    this->innerThread = std::move(other.innerThread);
    this->epollFd = other.epollFd;

    other.epollFd = -1;
}

WorkingThread::~WorkingThread() {
    innerThread.join();
    if (epollFd != -1) {
        close(epollFd);
    }
    //TODO: implement all operations to safely close all descriptors
}

void WorkingThread::start() {
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
                    int flags = fcntl(fd, F_GETFL, 0);
                    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
                        std::cerr << "Error setting socket to nonblocking mode: " << strerror(errno) << std::endl;
                    }

                    epoll_event event{};
                    event.events = EPOLLIN;
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
                handleSocketEvent(events[i].data.fd, events[i].events);
            }
        }
    });
}

int WorkingThread::getConnectionsCount() const {
    return connectionsCount;
}

void WorkingThread::addConnection(int fd) {
    while(!fdQueue.push(fd)){}
}

void WorkingThread::handleSocketEvent(int fd, uint32_t events) {
    if (events & EPOLLHUP) {
        close(fd);
        return;
    }

    if ((events & EPOLLIN)) {
        Request request(fd, epollFd, *this);
        request.handleRequest();
        return;
    }
}

void WorkingThread::decrementConnectionsCount() {
    --connectionsCount;
}

void WorkingThread::stop() {
    stopRequested = true;
}
