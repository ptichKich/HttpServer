#include "../include/linuxHeaders.hpp"
#include "../include/stdHeaders.hpp"
#include "../include/HttpServer.hpp"

static const std::string correctGetRequestText = "GET /index.html HTTP/1.1\r\n"
                                                 "Host: 127.0.0.1\r\n"
                                                 "Connection: keep-alive\r\n\r\n";

static const std::string badRequestText = "GET /index.html HTTP/1.1\r\n"
                                          "Hosl: 127.0.0.1\r\n"
                                          "Connection: keep-alive\r\n\r\n";

static const std::string notFoundGetRequestText = "GET /Index.html HTTP/1.1\r\n"
                                                  "Host: 127.0.0.1\r\n"
                                                  "Connection: keep-alive\r\n\r\n";

static const std::string correctPostRequestText = "POST /submit HTTP/1.1\r\n"
                                                  "Host: 127.0.0.1\r\n"
                                                  "Content-Type: application/json\r\n"
                                                  "Content-Length: 26\r\n\r\n"
                                                  "{\"username\":\"Alexandr\", \"age\":27}\r\n";

class Test final {
   int sockFd;
   int epollFd;

   std::ostringstream resultsStream;
public:
    Test() {
        sockFd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockFd == -1) {
            std::cerr << "Error creating client";
            exit(1);
        }

        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(60000);
        if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0) {
            std::cerr << "Wrong server address" << std::endl;
            close(sockFd);
            exit(1);
        }

        if (connect(sockFd, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) {
            std::cerr << "Error connecting to the server" << std::endl;
            close(sockFd);
            exit(1);
        }

        int flags = fcntl(sockFd, F_GETFL, 0);
        if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) < 0) {
            std::cerr << "Error setting socket to nonblocking mode: " << strerror(errno) << std::endl;
        }

        epollFd = epoll_create1(0);
        if (epollFd == -1) {
            std::cerr << "Error creating epoll: " << strerror(errno) << std::endl;
            exit(1);
        }

        epoll_event event{};
        event.events = EPOLLIN;
        event.data.fd = sockFd;

        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, sockFd, &event) == -1) {
            close(epollFd);
            exit(1);
        }
    }

    ~Test() {
        clearResources();
    }

    void run() {
        for (int i=0; i<100; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sendCorrectGetRequest();
            auto response = waitForResponse();

            assert(checkOkResponse(response));
        }

        resultsStream << std::this_thread::get_id() << " TEST checking (200 OK) for GET passed" << std::endl;

        for (int i=0; i<100; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sendIncorrectGetRequest();
            auto response = waitForResponse();

            assert(checkBadRequestResponse(response));
        }

        resultsStream << std::this_thread::get_id() << " Test checking (400 Bad Request) passed" << std::endl;

        for (int i=0; i<100; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sendCorrectPostRequest();
            auto response = waitForResponse();

            assert(checkOkPostResponse(response));
        }

        resultsStream << std::this_thread::get_id() << " TEST checking (200 OK) for POST passed" << std::endl;

        for (int i=0; i<100; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sendNotFoundGetRequest();
            auto response = waitForResponse();

            assert(checkNotFoundResponse(response));
        }

        resultsStream << std::this_thread::get_id() << " TEST checking (404 Not Found) for GET passed" << std::endl;
    }

    std::string getResults() {
        return resultsStream.str();
    }

private:
    void sendCorrectGetRequest() {
        if (send(sockFd, correctGetRequestText.c_str(), correctGetRequestText.size(), 0) == -1) {
            std::cerr << "Error sending correct GET request " << strerror(errno) << std::endl;
            clearResources();
            exit(1);
        }
    }

    void sendCorrectPostRequest() {
        if (send(sockFd, correctPostRequestText.c_str(), correctPostRequestText.size(), 0) == -1) {
            std::cerr << "Error sending correct POST request " << strerror(errno) <<  std::endl;
            clearResources();
            exit(1);
        }
    }

    void sendIncorrectGetRequest() {
        if (send(sockFd, badRequestText.c_str(), badRequestText.size(), 0) == -1) {
            std::cerr << "Error sending incorrect GET request " << strerror(errno) <<  std::endl;
            clearResources();
            exit(1);
        }
    }

    void sendNotFoundGetRequest() {
        if (send(sockFd, notFoundGetRequestText.c_str(), notFoundGetRequestText.size(), 0) == -1) {
            std::cerr << "Error sending not found GET request " << strerror(errno) <<  std::endl;
            clearResources();
            exit(1);
        }
    }

    std::string waitForResponse() {
        epoll_event events[1];
        int numEvents = epoll_wait(epollFd, events, 1, -1);
        if (numEvents == -1) {
            std::cerr << "Error epoll wait: " << strerror(errno) << std::endl;
            clearResources();
            return "";
        }

        char buffer[1024];
        ssize_t bytesRead;
        std::string response;

        while((bytesRead = recv(sockFd, buffer, sizeof(buffer) - 1, 0) ) > 0) {
            buffer[bytesRead] = '\0';
            response += buffer;
        }
        return response;
    }

    bool checkOkResponse(const std::string& response) {
        if (response.find("HTTP/1.1 200 OK") != std::string::npos) {
            return true;
        }

        return false;
    }

    bool checkBadRequestResponse(const std::string& response) {
        if (response.find("HTTP/1.1 400 Bad Request") != std::string::npos) {
            return true;
        }

        return false;
    }

    bool checkOkPostResponse(const std::string& response) {
        if (checkOkResponse(response) && response.find("Hello, Alexandr") != std::string::npos) {
            return true;
        }

        return false;
    }

    bool checkNotFoundResponse(const std::string& response) {
        if (response.find("HTTP/1.1 404 Not Found") != std::string::npos) {
            return true;
        }

        return false;
    };

    void clearResources() {
        close(sockFd);
        close(epollFd);
    }
};

HttpServer server(60000);

void signalHandler(int signal) {
    if (signal == SIGINT) {

        std::cout << "Shutting down, signal SIGINT received.\n";
        server.stop();
    }
}

int main() {
    std::signal(SIGINT, signalHandler);

    std::thread serverThread([](){
        server.run();
    });

    //to simplify the test just wait for 5 seconds before server starts
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::vector<std::future<std::string>> testsResults;
    for (int i=0; i<1000; ++i) {
        auto fut = std::async([](){
            Test test;
            test.run();

            return test.getResults();
        });
        testsResults.push_back(std::move(fut));
    }

    for (auto& result: testsResults) {
        std::cout << result.get() << std::endl;
    }

    serverThread.join();
}