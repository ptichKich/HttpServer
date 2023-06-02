#include "../include/Request.hpp"

static std::string okResponseGetFileContent = "<!DOCTYPE html>\n"
                                              "<html>\n"
                                              "<head>\n"
                                              "  <title>Greeting Page</title>\n"
                                              "</head>\n"
                                              "<body>\n"
                                              "  <h1>Hello, username. Introduce yourself</h1>\n"
                                              "  <p>...</p>\n"
                                              "</body>\n"
                                              "</html>";

static std::string okResponsePostContent = "<!DOCTYPE html>\n"
                                           "<html>\n"
                                           "<head>\n"
                                           "    <title>Greetings</title>\n"
                                           "</head>\n"
                                           "<body>\n"
                                           "    <h1 id=\"greeting\"></h1>\n"
                                           "\n"
                                           "    <script>\n"
                                           "        var greeting = \"greetingStr\"\n"
                                           "        document.getElementById(\"greeting\").innerText = greeting;\n"
                                           "    </script>\n"
                                           "</body>\n"
                                           "</html>";


Request::Request(int clientFd, int epollFd, WorkingThread &thr) :
            workingThreadRef(thr), clientFd(clientFd), epollFd(epollFd) {
}

void Request::handleRequest() {
    std::string requestBody = getRequestBody();
    auto requestContent = parseRequestBody(std::move(requestBody));

    if (!requestContent.empty()) {
        std::string firstLine = requestContent[0];
        std::istringstream issFirstLine(firstLine);
        std::string method, path, hostHeaderBegin;
        issFirstLine >> method >> path;

        std::istringstream issSecondLine(requestContent[1]);
        issSecondLine >> hostHeaderBegin;

        if (method == "GET") {
            handleGetRequest(path, hostHeaderBegin);
        }

        if (method == "POST") {
            handlePostRequest(requestContent);
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
        deleteFdFromEpollAndClose();
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

void Request::handleGetRequest(const std::string &path, const std::string& hostHeaderBegin) {
    std::ostringstream response;

    if (hostHeaderBegin != "Host:") {
        response << "HTTP/1.1 400 Bad Request\r\n";
        response << "Content-Type: text/plain\r\n";
        response << "Content-Length: 11\r\n";
        response << "\r\n\r\n";
        response << "Bad Request\n";

        sendResponse(response.str());
    } else if (path != "/index.html") {
        response << "HTTP/1.1 404 Not Found\r\n";
        response << "Content-Type: text/plain\r\n";
        response << "Content-Length: 11\r\n";
        response << "\r\n\r\n";
        response << "Not Found\n";

        sendResponse(response.str());
    } else {
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";
        response << "Content-Length: " << okResponseGetFileContent.size() << "\r\n";
        response << "\r\n\r\n";
        response << okResponseGetFileContent;

        sendResponse(response.str());
    }
}

void Request::handlePostRequest(const std::vector<std::string> &requestContent) {
    auto extractJsonValue = [&](const std::string& jsonStr, const std::string& key) -> std::string {
        std::string searchString = "\"" + key + "\":\"";
        size_t pos = jsonStr.find(searchString);
        if (pos != std::string::npos) {
            pos += searchString.length();
            size_t endPos = jsonStr.find("\"", pos);
            if (endPos != std::string::npos) {
                std::string value = jsonStr.substr(pos, endPos - pos);
                return value;
            }
        }
        return "";
    };

    const std::string& jsonContent = requestContent.back();
    auto name = extractJsonValue(jsonContent, "username");
    auto ageString = extractJsonValue(jsonContent, "age");

    std::string greeting = "\"Hello, " + name + ", now I know your age is " + ageString + "\"";
    std::ostringstream response;

    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: text/html\r\n";

    static std::string greetingStr("\"greetingStr\"");
    auto okResponsePostContentCopy = okResponsePostContent;
    auto pos = okResponsePostContentCopy.find(greetingStr);

    if (pos != std::string::npos) {
        okResponsePostContentCopy.replace(pos, greetingStr.length(), greeting);
    } else {
        std::cout << "Something went wrong replacing string \n";
        deleteFdFromEpollAndClose();
        exit(1);
    }


    response << "Content-Length: " << okResponsePostContentCopy.size() << "\r\n";
    response << "\r\n\r\n";
    response << okResponsePostContentCopy;

    sendResponse(response.str());
}

void Request::sendResponse(std::string &&response) {
    ssize_t totalSent = 0;
    ssize_t dataSize = response.size();

    while (totalSent < dataSize) {
        ssize_t bytesSent = send(clientFd, response.c_str() + totalSent, dataSize - totalSent, 0);
        if (bytesSent == -1) {
            std::cerr << "Error during data send." << std::endl;
            deleteFdFromEpollAndClose();
            exit(1);
        }
        totalSent += bytesSent;
    }
}

void Request::deleteFdFromEpollAndClose() {
    int result = epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, 0);
    if (result == -1) {
        std::cerr << "Error removing fd from epoll" << std::endl;
    }
    close(clientFd);
    workingThreadRef.decrementConnectionsCount();
}