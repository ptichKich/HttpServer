#include "../include/stdHeaders.hpp"

#include "../include/ThreadPool.hpp"
#include "../include/HttpServer.hpp"

HttpServer::HttpServer(int port) : threadPool(10) {
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        exit(1);
    }

    // Bind the socket to a port.
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
        closeSocketAndExit();
    }


    if (fcntl(sockFd, F_SETFL, O_NONBLOCK) < 0) {
        std::cerr << "Error setting socket to nonblocking mode: " << strerror(errno) << std::endl;
        closeSocketAndExit();
    }

    listen(sockFd, SOMAXCONN);
}

void HttpServer::run() {
    threadPool.launch();
    while(!stopRequested) {
        int fd = accept(sockFd, NULL, NULL);
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

void HttpServer::stop() {
    stopRequested = true;
}

void HttpServer::closeSocketAndExit() {
    close(sockFd);
    exit(1);
}