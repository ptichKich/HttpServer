#include <csignal>
#include "../include/stdHeaders.hpp"
#include "../include/HttpServer.hpp"


HttpServer server(60000);

void signalHandler(int signal) {
    if (signal == SIGINT) {

        std::cout << "Shutting down, signal SIGINT received.\n";
        server.stop();
    }
}

int main() {

    std::signal(SIGINT, signalHandler);

    return 0;
}
