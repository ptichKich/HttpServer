#include <csignal>
#include "HttpServer.hpp"

HttpServer server(60000);

void signalHandler(int signal) {
    if (signal == SIGINT) {
        server.stop();
    }
}

int main() {

    std::signal(SIGINT, signalHandler);
    server.Run();

    return 0;
}
