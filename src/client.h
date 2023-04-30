#pragma once

#include <boost/asio.hpp>
#include <cstdint>
#include <iostream>
#include <istream>
#include <limits>
#include <string>

class Client {
   public:
    Client(boost::asio::ip::tcp::socket&& socket);

   private:
    void Register();

    std::uint64_t ReadRegistrationResponse();

    void Poll();

   private:
    uint64_t id_;
    std::string username_;
    boost::asio::ip::tcp::socket socket_;
};

template <typename Int, typename InputPredicate>
auto SafeIntInput(Int& var, InputPredicate input_predicate,
                  const std::string& error_message) {
    while ((std::cout << "> ") &&
           (!(std::cin >> var) || !input_predicate(var))) {
        std::cout << error_message << std::endl;
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    }
}
