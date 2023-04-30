#include <csignal>
#include <cstdint>
#include <exception>
#include <iostream>

#include "db_manager.h"
#include "offer.h"
#include "server.h"

std::atomic<uint64_t> Offer::offer_id_ = GetDBManager().GetMaxId("Offer");
std::atomic<uint64_t> Deal::deal_id_ = GetDBManager().GetMaxId("Deal");

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
