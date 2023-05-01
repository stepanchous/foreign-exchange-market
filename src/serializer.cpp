#include "serializer.h"

#include <cstdint>
#include <string>

#include "common.h"
#include "db_manager.h"
#include "json.h"
#include "user_data.h"

using nlohmann::json;

std::string Serializer::RegisterUser(const std::string& username,
                                     size_t pw_hash) {
    json registration_confirmation;
    auto user_id = market_.RegisterUser(username, pw_hash);
    registration_confirmation[json_field::TYPE] = requests::REG_CONFIRMATION;
    registration_confirmation[json_field::SUCCESS] = user_id.has_value();
    if (user_id.has_value()) {
        registration_confirmation[json_field::USER_ID] = *user_id;
    } else {
        registration_confirmation[json_field::USER_ID] = nullptr;
    }

    return registration_confirmation.dump();
}

std::string Serializer::Login(const std::string& username, size_t pw_hash) {
    json response;
    auto user_id = GetDBManager().GetUserId(username, pw_hash);
    response[json_field::TYPE] = requests::LOGIN;
    response[json_field::SUCCESS] = user_id.has_value();
    if (user_id.has_value()) {
        response[json_field::USER_ID] = *user_id;
    } else {
        response[json_field::USER_ID] = nullptr;
    }

    return response.dump();
}

std::string Serializer::GetActiveOffers(uint64_t user_id) const {
    json response;
    const std::set<std::shared_ptr<Offer>, std::less<>>& active_offers =
        market_.GetActiveOffers(user_id);
    response[json_field::TYPE] = requests::ACTIVE_OFFERS;
    response[json_field::BUY] = json::array();
    response[json_field::SELL] = json::array();
    for (const auto& offer : active_offers) {
        response[OfferTypeToString(offer->GetType())].push_back(
            json({{json_field::OFFER_ID, offer->GetId()},
                  {json_field::PRICE, offer->GetPrice()},
                  {json_field::AMOUNT, offer->GetAmount()}}));
    }

    return response.dump();
}

std::string Serializer::GetClosedDeals(uint64_t user_id) const {
    json response;
    const std::set<std::shared_ptr<Deal>, std::less<>>& closed_deals =
        market_.GetClosedDeals(user_id);
    response[json_field::TYPE] = requests::CLOSED_DEALS;
    response[json_field::BUY] = json::array();
    response[json_field::SELL] = json::array();
    response[json_field::BUY_SELL] = json::array();
    for (const auto& deal : closed_deals) {
        if (deal->GetBuyer() == deal->GetSeller()) {
            response[json_field::BUY_SELL].push_back(
                {{json_field::PRICE, deal->GetPrice()},
                 {json_field::AMOUNT, deal->GetAmount()}});
            continue;
        }
        response[deal->GetBuyer() == user_id ? json_field::BUY
                                             : json_field::SELL]
            .push_back(json({{json_field::PRICE, deal->GetPrice()},
                             {json_field::AMOUNT, deal->GetAmount()}}));
    }

    return response.dump();
}

std::string Serializer::GetBalance(uint64_t user_id) const {
    json response;
    Balance balance = market_.GetUserBalance(user_id);
    response[json_field::TYPE] = requests::BALANCE;
    response[json_field::USD] = balance.usd;
    response[json_field::RUB] = balance.rub;

    return response.dump();
}

std::string Serializer::OfferTypeToString(OfferType offer_type) {
    switch (offer_type) {
        case OfferType::BUY:
            return json_field::BUY;
            break;
        case OfferType::SELL:
            return json_field::SELL;
            break;
    }
    return "";
}

std::string Serializer::GetQuotes() {
    json response;
    std::optional<int> quote = market_.GetQuote();
    AskBidQuotesInfo ask_bid_quotes_info = market_.GetAskBidQuotes();
    response[json_field::TYPE] = requests::QUOTES;
    if (quote.has_value()) {
        response[json_field::QUOTE] = *quote;
    } else {
        response[json_field::QUOTE] = nullptr;
    }
    if (ask_bid_quotes_info.ask_quote.has_value()) {
        response[json_field::ASK_QUOTE] = *ask_bid_quotes_info.ask_quote;
    } else {
        response[json_field::ASK_QUOTE] = nullptr;
    }
    if (ask_bid_quotes_info.bid_quote.has_value()) {
        response[json_field::BID_QUOTE] = *ask_bid_quotes_info.bid_quote;
    } else {
        response[json_field::BID_QUOTE] = nullptr;
    }
    if (ask_bid_quotes_info.spread.has_value()) {
        response[json_field::SPREAD] = *ask_bid_quotes_info.spread;
    } else {
        response[json_field::SPREAD] = nullptr;
    }

    return response.dump();
}

void Serializer::PostOffer(uint64_t user_id, OfferType offer_type, int price,
                           size_t amount) {
    market_.PostOffer(user_id, offer_type, price, amount);
}

std::string Serializer::CancelOffer(uint64_t user_id, uint64_t offer_id) {
    json response;
    bool is_deleted = market_.RemoveOffer(user_id, offer_id);
    response[json_field::TYPE] = requests::CANCEL;
    response[json_field::SUCCESS] = is_deleted;

    return response.dump();
}

Serializer& GetSerializer() {
    static Serializer responder;
    return responder;
}
