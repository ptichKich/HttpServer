#pragma once

#include "stdHeaders.hpp"
#include "linuxHeaders.hpp"
#include "WorkingThread.hpp"

class Request final {
public:
    Request(int clientFd, int epollFd, WorkingThread& thr);
    void handleRequest();

private:

    std::string getRequestBody();
    std::vector<std::string> parseRequestBody(std::string&& request);
    void handleGetRequest(const std::string& path, const std::string& hostHeaderBegin);

    void handlePostRequest(const std::vector<std::string>& requestContent);
    void sendResponse(std::string&& response);

    void deleteFdFromEpollAndClose();

    int clientFd;
    int epollFd;
    WorkingThread& workingThreadRef;
};

