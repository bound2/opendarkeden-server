#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

#include <netinet/in.h>
#include <sys/socket.h>

#include "CooperativeThread.h"
#include "ServerShutdown.h"

// Small stand-in processes for testing the REAL container supervisor without
// touching a running deployment or requiring a populated world database.
int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "db") {
        int listener = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 || listen(listener, 1) != 0)
            return 2;
        socklen_t size = sizeof(address);
        getsockname(listener, reinterpret_cast<sockaddr*>(&address), &size);
        std::ofstream("db.port") << ntohs(address.sin_port);
        int client = accept(listener, nullptr, nullptr);
        close(client);
        close(listener);
        return 0;
    }
    std::string name = std::filesystem::path(argv[0]).filename();
    struct sigaction action {};
    action.sa_handler = ServerShutdown::request;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGTERM, &action, nullptr) != 0)
        return 2;
    std::ofstream(name + ".ready") << "ready\n";
    CooperativeThread worker;
    worker.start([](std::stop_token token) {
        while (!token.stop_requested())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    });
    while (!ServerShutdown::isRequested())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    worker.requestStop();
    worker.join();
    if (name == "gameserver" &&
        (std::filesystem::exists("loginserver.stopped") || std::filesystem::exists("sharedserver.stopped")))
        return 3;
    std::ofstream(name + ".stopped") << "joined\n";
    return 0;
}
