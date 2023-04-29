#include <cstdint>
#include <string>

#include "common.h"
#include "json.h"
#include "serializer.h"
#include "user_data.h"

using nlohmann::json;

std::string Serializer::RegisterUser(const std::string& username) {
    json registration_confirmation;
    uint64_t user_id = market_.RegisterUser(username);
    registration_confirmation[json_field::TYPE] = requests::REG_CONFIRMATION;
    registration_confirmation[json_field::SUCCESS] = true;
    registration_confirmation[json_field::USER_ID] = user_id;
    return registration_confirmation.dump();
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
    for (const auto& deal : closed_deals) {
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
        default:
            return "";
    }
}

void Serializer::PostOffer(uint64_t user_id, OfferType offer_type, int price,
                           size_t amount) {
    market_.PostOffer(user_id, offer_type, price, amount);
}

Serializer& GetResponder() {
    static Serializer responder;
    return responder;
}
