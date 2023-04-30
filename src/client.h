#pragma once

#include <boost/asio.hpp>
#include <cstdint>
#include <iostream>
#include <istream>
#include <limits>
#include <string>

// Class that provides client front end for users
// communication with server
class Client {
   public:
    Client(boost::asio::ip::tcp::socket&& socket);

   private:
    struct AuthInfo {
        std::string username;
        std::string password;
    };

    void ProcessAuthentication();

    void Register();

    void Login();

    std::optional<uint64_t> ReadAuthResponse();

    void Poll();

    AuthInfo GetAuthInfo();

   private:
    uint64_t id_;
    boost::asio::ip::tcp::socket socket_;
};

// Safe int input function that checks input
// against provided predicate
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
