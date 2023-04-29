#include "request_handler.h"

#include <algorithm>
#include <cstdint>
#include <iostream>

#include "client.h"
#include "common.h"

using namespace boost::asio;
using nlohmann::json;

RequestHandler::RequestHandler(ip::tcp::socket& socket,
                               const std::string& request_type,
                               uint64_t user_id)
    : socket_(socket), request_type_(request_type), user_id_(user_id) {}

void RequestHandler::Handle() { PrintResult(SendRequest()); }

nlohmann::json RequestHandler::ReadResponse() {
    streambuf buff;
    read_until(socket_, buff, "\0");
    std::istream input(&buff);
    std::string line(std::istreambuf_iterator<char>(input), {});
    json response = json::parse(std::move(line));
    return response;
}

json IdOnlyRequestHandler::SendRequest() {
    json request;
    request[json_field::TYPE] = request_type_;
    request[json_field::USER_ID] = user_id_;

    auto request_str = request.dump();
    write(socket_, buffer(request_str, request_str.size()));

    return ReadResponse();
}

void GetQuotesHandler::PrintResult(const json& response) {
    std::cout << "Quotes\n";
    std::cout << "    Last Deal Price: "
              << NullableIntToString(response.at(json_field::QUOTE)) << '\n';
    std::cout << "    Ask quote      : "
              << NullableIntToString(response.at(json_field::ASK_QUOTE))
              << '\n';
    std::cout << "    Bid quote      : "
              << NullableIntToString(response.at(json_field::BID_QUOTE))
              << '\n';
    std::cout << "    Spread         : "
              << NullableIntToString(response.at(json_field::SPREAD)) << '\n';
}

std::string GetQuotesHandler::NullableIntToString(const json& nullable_int) {
    if (nullable_int.is_null()) {
        return "none";
    } else {
        return std::to_string(nullable_int.get<int>());
    }
}

void GetBalanceRequest::PrintResult(const json& response) {
    std::cout << "Your Balance\n";
    std::cout << "    USD: " << response.at(json_field::USD) << '\n';
    std::cout << "    RUB: " << response.at(json_field::RUB) << std::endl;
}

void GetActiveOffersRequest::PrintResult(const json& response) {
    std::cout << "Your Active Offers\n";
    if (!response.at(json_field::BUY).empty()) {
        std::cout << "     Buy offers:\n";
        for (const auto& buy_offer : response.at(json_field::BUY)) {
            std::cout << "        Offer id: "
                      << buy_offer.at(json_field::OFFER_ID) << '\n';
            std::cout << "        Amount  : "
                      << buy_offer.at(json_field::AMOUNT) << '\n';
            std::cout << "        Price   : " << buy_offer.at(json_field::PRICE)
                      << '\n'
                      << std::endl;
        }
    }
    if (!response.at(json_field::SELL).empty()) {
        std::cout << "     Sell offers:\n";
        for (const auto& sell_offer : response.at(json_field::SELL)) {
            std::cout << "        Offer id: "
                      << sell_offer.at(json_field::OFFER_ID) << '\n';
            std::cout << "        Amount  : "
                      << sell_offer.at(json_field::AMOUNT) << '\n';
            std::cout << "        Price   : "
                      << sell_offer.at(json_field::PRICE) << '\n'
                      << std::endl;
        }
    }

    if (response.at(json_field::BUY).empty() &&
        response.at(json_field::SELL).empty()) {
        std::cout << "You do not have any active offers." << std::endl;
    }
}

void GetClosedDealsRequest::PrintResult(const json& response) {
    std::cout << "Your Active Offers\n";
    if (!response.at(json_field::BUY).empty()) {
        std::cout << "     Buy deals:\n";
        for (const auto& buy_offer : response.at(json_field::BUY)) {
            std::cout << "        Amount  : "
                      << buy_offer.at(json_field::AMOUNT) << '\n';
            std::cout << "        Price   : " << buy_offer.at(json_field::PRICE)
                      << '\n'
                      << std::endl;
        }
    }

    if (!response.at(json_field::SELL).empty()) {
        std::cout << "     Sell deals:\n";
        for (const auto& sell_offer : response.at(json_field::SELL)) {
            std::cout << "        Amount  : "
                      << sell_offer.at(json_field::AMOUNT) << '\n';
            std::cout << "        Price   : "
                      << sell_offer.at(json_field::PRICE) << '\n'
                      << std::endl;
        }
    }

    if (response.at(json_field::BUY).empty() &&
        response.at(json_field::SELL).empty()) {
        std::cout << "You do not have any closed deals." << std::endl;
    }
}

void PostOfferRequest::GatherPrerequisites() {
    std::cout << "Enter offer type:" << '\n';
    std::cout << "    1) Post buy offer" << '\n';
    std::cout << "    2) Post sell offer" << '\n';

    SafeIntInput(
        offer_side_, [](int var) { return var == 1 || var == 2; },
        "Invalid option. Try again.");

    std::cout << "Enter amount:" << std::endl;
    SafeIntInput(
        amount_, [](int amount) { return amount > 0; },
        "Invalid amount. Try again.");

    std::cout << "Enter price: " << '\n';
    SafeIntInput(
        price_, []([[maybe_unused]] int price) { return true; },
        "Invalid price. Try again.");
}

json PostOfferRequest::SendRequest() {
    GatherPrerequisites();

    json request;
    request[json_field::TYPE] = requests::POST_OFFER;
    request[json_field::USER_ID] = user_id_;
    request[json_field::OFFER_SIDE] =
        offer_side_ == 1 ? json_field::BUY : json_field::SELL;
    request[json_field::AMOUNT] = amount_;
    request[json_field::PRICE] = price_;

    auto request_str = request.dump();
    write(socket_, buffer(request_str, request_str.size()));

    return ReadResponse();
}

void PostOfferRequest::PrintResult(const json& response) {
    std::cout << response.get<std::string>() << std::endl;
}

void CancleOfferRequest::GatherPrerequisites() {
    std::cout << "Enter offer id" << std::endl;
    SafeIntInput(
        offer_id_, [](int var) { return var >= 0; },
        "Invalid offer id. Try again.");
}

json CancleOfferRequest::SendRequest() {
    GatherPrerequisites();

    json request;
    request[json_field::TYPE] = requests::CANCEL;
    request[json_field::USER_ID] = user_id_;
    request[json_field::OFFER_ID] = offer_id_;

    auto request_str = request.dump();
    write(socket_, buffer(request_str, request_str.size()));

    return ReadResponse();
}

void CancleOfferRequest::PrintResult(const json& response) {
    std::cout << (response.at(json_field::SUCCESS)
                      ? "Offer has been canceled successfully."
                      : "Unable to cancle offer. Please make sure it is active "
                        "buy checking "
                        "active offers list.")
              << std::endl;
}

std::unique_ptr<RequestHandler> MakeRequest(RequestType type,
                                            ip::tcp::socket& socket,
                                            uint64_t user_id) {
    switch (type) {
        case RequestType::POST_OFFER:
            return std::make_unique<PostOfferRequest>(
                socket, requests::POST_OFFER, user_id);

        case RequestType::CANCEL_OFFER:
            return std::make_unique<CancleOfferRequest>(
                socket, requests::CANCEL, user_id);

        case RequestType::GET_QUOTES:
            return std::make_unique<CancleOfferRequest>(
                socket, requests::QUOTES, user_id);

        case RequestType::GET_BALANCE:
            return std::make_unique<GetBalanceRequest>(
                socket, requests::BALANCE, user_id);

        case RequestType::GET_ACTIVE:
            return std::make_unique<GetActiveOffersRequest>(
                socket, requests::ACTIVE_OFFERS, user_id);

        case RequestType::GET_CLOSED:
            return std::make_unique<GetClosedDealsRequest>(
                socket, requests::CLOSED_DEALS, user_id);
    }
}
