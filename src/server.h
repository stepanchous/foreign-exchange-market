#pragma once

#include <boost/asio/io_service.hpp>

#include "session.h"

class Server {
   public:
    Server(boost::asio::io_service& io_service);

    void HandleAccept(Session* new_session,
                      const boost::system::error_code& error);

    ~Server();

   private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
};
