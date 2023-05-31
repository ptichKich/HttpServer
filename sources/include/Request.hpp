#pragma once

#include "stdHeaders.hpp"
#include "linuxHeaders.hpp"
#include "WorkingThread.hpp"

class Request {
    enum class RequestType: uint8_t {
        GET,
        POST,
    };
public:
    Request(int clientFd, int epollFd, WorkingThread& thr);

    void handleRequest();

    std::string getRequestBody();

    std::vector<std::string> parseRequestBody(std::string&& request);

    void handleGetRequest(std::string&& path);

    void sendResponse(std::string&& response);


private:
    int clientFd;
    int epollFd;
    WorkingThread& workingThreadRef;
};

