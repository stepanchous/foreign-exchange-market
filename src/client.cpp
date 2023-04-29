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
            option, [](int& var) { return var >= 1 && var <= 7; },
            "Invalid option. Try again.");

        switch (option) {
            case 1:
                PrintPostOfferResponse(PostOffer());
                break;
            case 2:
                PrintCancelOfferResponse(CancelOffer());
                break;
            case 3:
                PrintQuotes(SendRequest(requests::QUOTES));
                break;
            case 4:
                PrintBalance(SendRequest(requests::BALANCE));
                break;
            case 5:
                PrintActiveOffers(SendRequest(requests::ACTIVE_OFFERS));
                break;
            case 6:
                PrintClosedDeals(SendRequest(requests::CLOSED_DEALS));
                break;
            case 7:
                std::cout << "Thank you for using StepStock Market!"
                          << std::endl;
                return;
        }
    }
}

nlohmann::json Client::SendRequest(const std::string& request_type) {
    json request;
    request[json_field::TYPE] = request_type;
    request[json_field::USER_ID] = id_;

    auto request_str = request.dump();
    write(socket_, buffer(request_str, request_str.size()));

    return ReadResponse();
}

nlohmann::json Client::ReadResponse() {
    streambuf buff;
    read_until(socket_, buff, "\0");
    std::istream input(&buff);
    std::string line(std::istreambuf_iterator<char>(input), {});
    json response = json::parse(std::move(line));
    return response;
}

json Client::PostOffer() {
    json request;
    request[json_field::TYPE] = requests::POST_OFFER;
    request[json_field::USER_ID] = id_;
    std::cout << "Enter offer type:" << '\n';
    std::cout << "    1) Post buy offer" << '\n';
    std::cout << "    2) Post sell offer" << '\n';

    int option;
    SafeIntInput(
        option, [](int var) { return var == 1 || var == 2; },
        "Invalid option. Try again.");

    request[json_field::OFFER_SIDE] =
        option == 1 ? json_field::BUY : json_field::SELL;

    std::cout << "Enter amount:" << std::endl;
    int amount;
    SafeIntInput(
        amount, [](int amount) { return amount > 0; },
        "Invalid amount. Try again.");

    std::cout << "Enter price: " << '\n';
    int price;
    SafeIntInput(
        price, []([[maybe_unused]] int price) { return true; },
        "Invalid price. Try again.");

    request[json_field::AMOUNT] = amount;
    request[json_field::PRICE] = price;

    auto request_str = request.dump();
    write(socket_, buffer(request_str, request_str.size()));

    return ReadResponse();
}

json Client::CancelOffer() {
    json request;
    std::cout << "Enter offer id" << std::endl;
    int offer_id;
    SafeIntInput(
        offer_id, [](int var) { return var >= 0; },
        "Invalid offer id. Try again.");

    request[json_field::TYPE] = requests::CANCEL;
    request[json_field::USER_ID] = id_;
    request[json_field::OFFER_ID] = offer_id;

    auto request_str = request.dump();
    write(socket_, buffer(request_str, request_str.size()));

    return ReadResponse();
}

void Client::PrintBalance(const json& balance_json) {
    std::cout << "Your Balance\n";
    std::cout << "    USD: " << balance_json.at(json_field::USD) << '\n';
    std::cout << "    RUB: " << balance_json.at(json_field::RUB) << std::endl;
}

void Client::PrintActiveOffers(const json& active_offers_json) {
    std::cout << "Your Active Offers\n";
    if (!active_offers_json.at(json_field::BUY).empty()) {
        std::cout << "     Buy offers:\n";
        for (const auto& buy_offer : active_offers_json.at(json_field::BUY)) {
            std::cout << "        Offer id: "
                      << buy_offer.at(json_field::OFFER_ID) << '\n';
            std::cout << "        Amount  : "
                      << buy_offer.at(json_field::AMOUNT) << '\n';
            std::cout << "        Price   : " << buy_offer.at(json_field::PRICE)
                      << '\n'
                      << std::endl;
        }
    }
    if (!active_offers_json.at(json_field::SELL).empty()) {
        std::cout << "     Sell offers:\n";
        for (const auto& sell_offer : active_offers_json.at(json_field::SELL)) {
            std::cout << "        Offer id: "
                      << sell_offer.at(json_field::OFFER_ID) << '\n';
            std::cout << "        Amount  : "
                      << sell_offer.at(json_field::AMOUNT) << '\n';
            std::cout << "        Price   : "
                      << sell_offer.at(json_field::PRICE) << '\n'
                      << std::endl;
        }
    }

    if (active_offers_json.at(json_field::BUY).empty() &&
        active_offers_json.at(json_field::SELL).empty()) {
        std::cout << "You do not have any active offers." << std::endl;
    }
}

void Client::PrintClosedDeals(const json& closed_deals_json) {
    std::cout << "Your Active Offers\n";
    if (!closed_deals_json.at(json_field::BUY).empty()) {
        std::cout << "     Buy deals:\n";
        for (const auto& buy_offer : closed_deals_json.at(json_field::BUY)) {
            std::cout << "        Amount  : "
                      << buy_offer.at(json_field::AMOUNT) << '\n';
            std::cout << "        Price   : " << buy_offer.at(json_field::PRICE)
                      << '\n'
                      << std::endl;
        }
    }

    if (!closed_deals_json.at(json_field::SELL).empty()) {
        std::cout << "     Sell deals:\n";
        for (const auto& sell_offer : closed_deals_json.at(json_field::SELL)) {
            std::cout << "        Amount  : "
                      << sell_offer.at(json_field::AMOUNT) << '\n';
            std::cout << "        Price   : "
                      << sell_offer.at(json_field::PRICE) << '\n'
                      << std::endl;
        }
    }

    if (closed_deals_json.at(json_field::BUY).empty() &&
        closed_deals_json.at(json_field::SELL).empty()) {
        std::cout << "You do not have any closed deals." << std::endl;
    }
}

void Client::PrintQuotes(const json& quotes_json) {
    std::cout << "Quotes\n";
    std::cout << "    Last Deal Price: "
              << NullableIntToString(quotes_json.at(json_field::QUOTE)) << '\n';
    std::cout << "    Ask quote      : "
              << NullableIntToString(quotes_json.at(json_field::ASK_QUOTE))
              << '\n';
    std::cout << "    Bid quote      : "
              << NullableIntToString(quotes_json.at(json_field::BID_QUOTE))
              << '\n';
    std::cout << "    Spread         : "
              << NullableIntToString(quotes_json.at(json_field::SPREAD))
              << '\n';
}

void Client::PrintPostOfferResponse(const json& response) {
    std::cout << response.get<std::string>() << std::endl;
}

void Client::PrintCancelOfferResponse(const json& response) {
    std::cout << (response.at(json_field::SUCCESS)
                      ? "Offer has been canceled successfully."
                      : "Unable to cancle offer. Please make sure it is active "
                        "buy checking "
                        "active offers list.")
              << std::endl;
}

std::string Client::NullableIntToString(const json& nullable_int) {
    if (nullable_int.is_null()) {
        return "none";
    } else {
        return std::to_string(nullable_int.get<int>());
    }
}
