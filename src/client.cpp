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
    ProcessAuthentication();
    Poll();
}

// Function is called at start up of client
// and process authentication wether it is
// registration or login
void Client::ProcessAuthentication() {
    std::cout << "Authentication required." << std::endl;
    std::cout << "    1) Register\n";
    std::cout << "    2) Login" << std::endl;
    int option;
    SafeIntInput(
        option, [](int var) { return var == 1 || var == 2; },
        "Invalid option. Try again.");
    switch (option) {
        case 1:
            Register();
            break;
        case 2:
            Login();
            break;
    }
}

void Client::Register() {
    AuthInfo auth_info = GetAuthInfo();

    json request;
    request[json_field::TYPE] = requests::REGISTRATION;
    request[json_field::USERNAME] = auth_info.username;
    request[json_field::PW_HASH] = std::hash<std::string>{}(auth_info.password);

    auto request_str = request.dump();
    write(socket_, buffer(request_str, request_str.size()));
    auto registration_result = ReadAuthResponse();
    if (!registration_result) {
        std::cout << "User with this username already exist. Try again."
                  << std::endl;
        ProcessAuthentication();
    } else {
        id_ = *registration_result;
    }
}

void Client::Login() {
    AuthInfo auth_info = GetAuthInfo();

    json request;
    request[json_field::TYPE] = requests::LOGIN;
    request[json_field::USERNAME] = auth_info.username;
    request[json_field::PW_HASH] = std::hash<std::string>{}(auth_info.password);

    auto request_str = request.dump();
    write(socket_, buffer(request_str, request_str.size()));
    auto registration_result = ReadAuthResponse();
    if (!registration_result) {
        std::cout << "Wrong username or password. Try again." << std::endl;
        ProcessAuthentication();
    } else {
        id_ = *registration_result;
    }
}

std::optional<uint64_t> Client::ReadAuthResponse() {
    streambuf buff;
    read_until(socket_, buff, "\0");
    std::istream input(&buff);
    std::string line(std::istreambuf_iterator<char>(input), {});
    json registration_response = json::parse(line);
    if (registration_response.at(json_field::SUCCESS)) {
        return registration_response.at(json_field::USER_ID);
    } else {
        return std::nullopt;
    }
}

// Main menu of application
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

Client::AuthInfo Client::GetAuthInfo() {
    std::string username;
    std::string password;
    std::cout << "Enter username." << std::endl;
    std::cout << "> ";
    std::cin >> username;
    std::cout << "Enter password." << std::endl;
    std::cout << "> ";
    std::cin >> password;

    return {.username = std::move(username), .password = std::move(password)};
}
