#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <exception>
#include <iostream>
#include <string>

#include "client.h"
#include "common.h"

using namespace boost::asio;

int main() {
    try {
        io_service io_service;
        ip::tcp::resolver resolver(io_service);
        ip::tcp::resolver::query query(ip::tcp::v4(), "127.0.0.1",
                                       std::to_string(port),
                                       ip::resolver_query_base::flags());
        ip::tcp::resolver::iterator it = resolver.resolve(query);

        ip::tcp::socket socket(io_service);
        socket.connect(*it);

        Client client(std::move(socket));
    } catch (std::exception& er) {
        std::cout << "ERROR: " << er.what() << std::endl;
    }
}
