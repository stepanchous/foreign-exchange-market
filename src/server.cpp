#include "server.h"

#include <boost/asio/placeholders.hpp>
#include <iostream>

#include "common.h"
#include "session.h"

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_service& io_service)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
    std::cout << "Server started." << '\n';
    std::cout << "Listening port: " << port << std::endl;
    Session* session = new Session(io_service_);
    acceptor_.async_accept(session->GetSocket(),
                           boost::bind(&Server::HandleAccept, this, session,
                                       boost::asio::placeholders::error));
}

Server::~Server() { std::cout << "\nServer shutdown" << std::endl; }

void Server::HandleAccept(Session* new_session,
                          const boost::system::error_code& error) {
    if (!error) {
        new_session->Start();
        new_session = new Session(io_service_);
        acceptor_.async_accept(
            new_session->GetSocket(),
            boost::bind(&Server::HandleAccept, this, new_session,
                        boost::asio::placeholders::error));
    } else {
        delete new_session;
    }
}
