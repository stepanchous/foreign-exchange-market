#include "market.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <queue>

#include "deal.h"
#include "offer.h"

bool operator<(const OfferQueue& lhs, const OfferQueue& rhs) {
    return lhs.price < rhs.price;
}
bool operator<(int lhs, const OfferQueue& rhs) { return lhs < rhs.price; }
bool operator<(const OfferQueue& lhs, int rhs) { return lhs.price < rhs; }

uint64_t Market::RegisterUser(const std::string& username) {
    auto user_id = GenerateUserId();
    user_id_to_user_data_.insert({user_id, UserData(username)});
    return user_id;
}

void Market::DepositUSD(uint64_t user_id, size_t usd_amount) {
    user_id_to_user_data_.at(user_id).DepositUSD(usd_amount);
}

void Market::DepositRUB(uint64_t user_id, size_t rub_amount) {
    user_id_to_user_data_.at(user_id).DepositRUB(rub_amount);
}

Balance Market::GetUserBalance(uint64_t user_id) const {
    return user_id_to_user_data_.at(user_id).GetBalance();
}

const std::set<std::shared_ptr<Offer>, std::less<>>& Market::GetActiveOffers(
    uint64_t user_id) const {
    return user_id_to_user_data_.at(user_id).GetActiveOffers();
}

const std::set<std::shared_ptr<Deal>, std::less<>>& Market::GetClosedDeals(
    uint64_t user_id) const {
    return user_id_to_user_data_.at(user_id).GetClosedDeals();
}

std::optional<int> Market::GetQuote() const { return quote_; }

AskBidQuotesInfo Market::GetAskBidQuotes() {
    AskBidQuotesInfo quotes_info = {
        .ask_quote = DetermineQuote(OfferType::BUY),
        .bid_quote = DetermineQuote(OfferType::SELL),
        .spread = std::nullopt};
    if (quotes_info.ask_quote && quotes_info.bid_quote) {
        quotes_info.spread = *quotes_info.bid_quote - *quotes_info.ask_quote;
    }

    return quotes_info;
}

uint64_t Market::PostOffer(uint64_t user_id, OfferType offer_type, int price,
                           size_t amount) {
    auto new_offer =
        std::make_shared<Offer>(user_id, offer_type, price, amount);
    user_id_to_user_data_.at(user_id).AddOffer(new_offer);
    switch (new_offer->GetType()) {
        case OfferType::SELL: {
            ProcessOffer(
                new_offer,
                [](std::set<OfferQueue, std::less<>>& offers, int price) {
                    return (--offers.end())->price >= price;
                },
                [](std::set<OfferQueue, std::less<>>& offers) {
                    return (--offers.end());
                });
            break;
        }
        case OfferType::BUY: {
            ProcessOffer(
                new_offer,
                [](std::set<OfferQueue, std::less<>>& offers, int price) {
                    return offers.begin()->price <= price;
                },
                [](std::set<OfferQueue, std::less<>>& offers) {
                    return offers.begin();
                });
            break;
        }
    }
    return new_offer->GetId();
}

bool Market::RemoveOffer(uint64_t user_id, uint64_t offer_id) {
    return user_id_to_user_data_.at(user_id).RemoveActiveOffer(offer_id);
}

void Market::AddActiveOffer(const std::shared_ptr<Offer>& offer) {
    std::set<OfferQueue, std::less<>>& offers =
        offer->GetType() == OfferType::SELL ? active_sell_offers_
                                            : active_buy_offers_;
    auto corresponding_price_queue = offers.find(offer->GetPrice());
    if (corresponding_price_queue == offers.end()) {
        offers.insert({offer->GetPrice(), {std::weak_ptr(offer)}});
    } else {
        corresponding_price_queue->offers.push_front(std::weak_ptr(offer));
    }
}

void Market::RegisterDeal(const std::shared_ptr<Deal>& deal) {
    UserData& buyer = user_id_to_user_data_.at(deal->GetBuyer());
    UserData& seller = user_id_to_user_data_.at(deal->GetSeller());

    buyer.DepositUSD(deal->GetAmount());
    buyer.WithdrawRUB(deal->GetAmount() * deal->GetPrice());

    seller.WithdrawUSD(deal->GetAmount());
    seller.DepositRUB(deal->GetAmount() * deal->GetPrice());

    buyer.AddDeal(deal);
    seller.AddDeal(deal);
}

void Market::UpdateQuote(const std::shared_ptr<Deal>& deal) {
    quote_ = deal->GetPrice();
}

std::optional<int> Market::DetermineQuote(OfferType offer_type) {
    std::optional<int> quote;
    auto& offers = offer_type == OfferType::SELL ? active_sell_offers_
                                                 : active_buy_offers_;

    while (!offers.empty() && !quote.has_value()) {
        auto best_offers =
            offer_type == OfferType::SELL ? --offers.end() : offers.begin();
        while (!best_offers->offers.empty() && !quote.has_value()) {
            std::weak_ptr<Offer>& best_offer = best_offers->offers.back();
            if (best_offer.expired()) {
                best_offers->offers.pop_back();
                if (best_offers->offers.empty()) {
                    offers.erase(best_offers);
                }
            } else {
                quote = best_offer.lock()->GetPrice();
            }
        }
    }

    return quote;
}

uint64_t Market::GenerateUserId() { return user_id_++; }
