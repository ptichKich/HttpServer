#include "../include/stdHeaders.hpp"
#include "../include/HttpServer.hpp"

static constexpr int port = 60000;
HttpServer server(port);

void signalHandler(int signal) {
    if (signal == SIGINT) {

        std::cout << "Shutting down, signal SIGINT received.\n";
        server.stop();
    }
}

int main() {
    std::signal(SIGINT, signalHandler);
    server.run();

    return 0;
}
