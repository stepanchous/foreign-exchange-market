#include <csignal>
#include <exception>
#include <iostream>

#include "server.h"

boost::asio::io_service io_service;

void signal_handler(int) { io_service.stop(); }

int main() {
    std::signal(SIGINT, signal_handler);
    try {
        Server server(io_service);
        io_service.run();
    } catch (std::exception& er) {
        std::cerr << "ERROR: " << er.what() << std::endl;
    }
}
