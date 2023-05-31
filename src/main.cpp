#include <csignal>
#include "include/HttpServer.hpp"


HttpServer server(60000);

void signalHandler(int signal) {
    if (signal == SIGINT) {

        std::cout << "Shutting down, signal SIGINT received.\n";
        server.stop();
    }
}

int main() {

    std::signal(SIGINT, signalHandler);
//    std::thread thr([](){
//        std::signal(SIGINT, signalHandler);
//        while(true) {
//            std::this_thread::sleep_for(std::chrono::seconds(1));
//        }
//    });
//    thr.detach();
    server.Run();

    return 0;
}
