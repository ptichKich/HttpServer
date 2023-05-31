#include "../include/Request.hpp"

Request::Request(int clientFd, int epollFd, WorkingThread &thr) :
            workingThreadRef(thr), clientFd(clientFd), epollFd(epollFd) {
}

void Request::handleRequest() {
    std::string requestBody = getRequestBody();
    auto requestContent = parseRequestBody(std::move(requestBody));

    if (!requestContent.empty()) {
        std::string firstLine = requestContent[0];
        std::istringstream issFirstLine(firstLine);
        std::string method;
        std::string path;
        issFirstLine >> method >> path;

        if (method == "GET") {
            handleGetRequest(std::move(path));
        }
    }
}

std::string Request::getRequestBody() {
    std::string request;
    char buffer[1024];
    ssize_t bytesRead;

    while((bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0) ) > 0) {
        buffer[bytesRead] = '\0';
        request += buffer;
    }

    if (bytesRead == 0) {
        int result = epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, 0);
        if (result == -1) {
            std::cerr << "Error removing fd from epoll" << std::endl;
        }
        close(clientFd);
        workingThreadRef.decrementConnectionsCount();
    }

    return request;
}

std::vector<std::string> Request::parseRequestBody(std::string &&request) {
    std::istringstream issRequest(std::move(request));
    std::vector<std::string> lines;

    std::string line;
    while (std::getline(issRequest, line, '\n')) {
        lines.push_back(line);
    }

    return lines;
}

void Request::handleGetRequest(std::string &&path) {
    std::ostringstream response;
    std::ostringstream ossResponseFileContent;

//        if (path == "/index.html") {
//            std::ifstream file("../html/index.html");
//            if (file) {
//                ossResponseFileContent << file.rdbuf();
//            }
//        }


    //std::string responseFileContent(ossResponseFileContent.str());

    std::string responseFileContent = "<!DOCTYPE html>\n"
                                      "<html>\n"
                                      "<head>\n"
                                      "    <title>Web page example</title>\n"
                                      "</head>\n"
                                      "<body>\n"
                                      "\n"
                                      "<h1>Web page example</h1>\n"
                                      "\n"
                                      "<form>\n"
                                      "    <label for=\"inputField\">Enter the text:</label>\n"
                                      "    <input type=\"text\" id=\"inputField\" name=\"inputField\">\n"
                                      "    <br><br>\n"
                                      "    <input type=\"submit\" value=\"Send\">\n"
                                      "</form>\n"
                                      "\n"
                                      "</body>\n"
                                      "</html>\n";

    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << responseFileContent.size() << "\r\n";
    response << "\r\n\r\n";
    response << responseFileContent;

    sendResponse(response.str());
}

void Request::sendResponse(std::string &&response) {
    //TODO send in async way
    ssize_t sentBytes = send(clientFd, response.c_str(), response.length(), 0);
    if (sentBytes < 0) {
        std::cerr << "Error sending response" << std::endl;
    }
}