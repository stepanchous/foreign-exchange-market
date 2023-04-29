#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind/bind.hpp>
#include <string>

class Session {
   public:
    Session(boost::asio::io_service& io_service);

    void Start();

    void HandleRead(const boost::system::error_code& error,
                    size_t bytes_transferred);

    void HandleWrite(const boost::system::error_code& error);

    boost::asio::ip::tcp::socket& GetSocket();

   private:
    boost::asio::ip::tcp::socket socket_;
    enum { max_length = 4096 };
    char data_[max_length];
};
