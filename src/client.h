#pragma once

#include <boost/asio.hpp>
#include <cstdint>
#include <iostream>
#include <istream>
#include <limits>
#include <string>

#include "json.h"

class Client {
   public:
    Client(boost::asio::ip::tcp::socket&& socket);

   private:
    void Register();

    std::uint64_t ReadRegistrationResponse();

    void Poll();

    nlohmann::json SendRequest(const std::string& request_type);

    nlohmann::json ReadResponse();

    nlohmann::json PostOffer();

    nlohmann::json CancelOffer();

   private:
    static void PrintBalance(const nlohmann::json& balance_json);

    static void PrintActiveOffers(const nlohmann::json& active_offers_json);

    static void PrintClosedDeals(const nlohmann::json& closed_deals_json);

    static void PrintQuotes(const nlohmann::json& quotes_json);

    static void PrintPostOfferResponse(const nlohmann::json& response);

    static void PrintCancelOfferResponse(const nlohmann::json& response);

    static std::string NullableIntToString(const nlohmann::json& nullable_int);

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
