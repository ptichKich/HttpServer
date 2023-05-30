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

#include "Request.hpp"
#include "ThreadPool.hpp"

class HttpServer {
public:
    HttpServer(int port): threadPool(10) {
        // Create a socket.
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
            exit(1);
        }

        // Bind the socket to a port.
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
            closeSocketAndExit();
        }


//        if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
//            std::cerr << "Error setting socket to nonblocking mode: " << strerror(errno) << std::endl;
//            closeSocketAndExit();
//        }

        // Listen for incoming connections.
        listen(sockfd, SOMAXCONN);

//        epollFd = epoll_create1(0);
//        if (epollFd == -1) {
//            std::cerr << "Error creating epoll: " << strerror(errno) << std::endl;
//            closeSocketAndExit();
//        }
//
//        epoll_event event{};
//        event.events = EPOLLIN;
//        event.data.fd = sockfd;
//
//        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, sockfd, &event) == -1) {
//            // TODO: write smth reasonable std::cerr << "Error: " << strerror(errno) << std::endl;
//            close(epollFd);
//            closeSocketAndExit();
//        }
    }

    void Run() {
        threadPool.launch();
        while(!stopRequested) {
            int fd = accept(sockfd, NULL, NULL);
            if (fd == -1)
            {
                if (errno == EAGAIN) {
                    continue;
                }

                std::cerr << "Error accepting: " << strerror(errno) << std::endl;
                closeSocketAndExit();
            }
            threadPool.addConnection(fd);
        }
        threadPool.stop();
    }

    void stop() {
        stopRequested = true;
    }

private:
    void closeSocketAndExit() {
        close(sockfd);
        exit(1);
    }

private:
    static constexpr int MAX_EVENTS = 10;
    int sockfd;

private:
    ThreadPool threadPool;
    std::atomic<bool> stopRequested{false};
};
