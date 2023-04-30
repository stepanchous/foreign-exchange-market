#include "client.h"

#include <boost/asio/buffer.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>

#include "common.h"
#include "json.h"
#include "request_handler.h"

using namespace boost::asio;
using nlohmann::json;

Client::Client(ip::tcp::socket&& socket) : socket_(std::move(socket)) {
    std::cout << "Welcome to StepStock Market!\n" << std::endl;
    Register();
    Poll();
}

void Client::Register() {
    std::string username;
    std::cout << "Enter username." << std::endl;
    std::cout << "> ";
    std::cin >> username;

    json request;
    request[json_field::TYPE] = requests::REGISTRATION;
    request[json_field::USERNAME] = username;

    auto request_str = request.dump();
    write(socket_, buffer(request_str, request_str.size()));
    id_ = ReadRegistrationResponse();
    username_ = std::move(username);
}

uint64_t Client::ReadRegistrationResponse() {
    streambuf buff;
    read_until(socket_, buff, "\0");
    std::istream input(&buff);
    std::string line(std::istreambuf_iterator<char>(input), {});
    json registration_response = json::parse(line);
    return registration_response.at(json_field::USER_ID);
}

void Client::Poll() {
    while (true) {
        std::cout << "\nEnter option:\n"
                     "    1) Post offer\n"
                     "    2) Cancel offer\n"
                     "    3) Get quotes\n"
                     "    4) Get balance\n"
                     "    5) Get active offers\n"
                     "    6) Get closed deals\n"
                     "    7) End session\n"
                  << std::endl;

        int option;
        SafeIntInput(
            option,
            [](int& var) {
                return var >= RequestType::FIRST &&
                       var <= RequestType::LAST + 1;
            },
            "Invalid option. Try again.");

        switch (option) {
            case RequestType::LAST + 1:
                std::cout << "Thank you for using StepStock Market!"
                          << std::endl;
                return;
            default:
                auto request_handler =
                    MakeRequest(static_cast<RequestType>(option), socket_, id_);
                if (request_handler) {
                    request_handler->Handle();
                } else {
                    std::cout << "Unknown request type." << std::endl;
                }
        }
    }
}
