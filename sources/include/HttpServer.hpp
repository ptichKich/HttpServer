#pragma once
#include "GenericServerIface.hpp"
#include "ThreadPool.hpp"

class HttpServer: public GenericServerIface {
public:
    explicit HttpServer(int port);
    ~HttpServer() override = default;

    void run() override;
    void stop() override;

private:
    void closeSocketAndExit();

private:
    int sockFd;
    ThreadPool threadPool;
    std::atomic<bool> stopRequested{false};
};
