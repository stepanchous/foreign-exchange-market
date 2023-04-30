#include "offer.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>

#ifndef TEST
#include "db_manager.h"
#endif  // !TEST

Offer::Offer(uint64_t owner_id, OfferType type, int price, size_t amount)
    : id_(GenerateId()),
      owner_id_(owner_id),
      type_(type),
      price_(price),
      amount_(amount),
      status_(OfferStatus::ACTIVE) {
#ifndef TEST
    GetDBManager().AddOffer(id_, owner_id_, type_, amount_, price_);
#endif  // !TEST
}

Deal Offer::MakeDeal(Offer& other) {
    if (this->type_ == other.type_) {
        throw std::logic_error("ERROR: Deals have same type");
    }

    Offer& buy_offer = this->type_ == OfferType::BUY ? *this : other;
    Offer& sell_offer = this->type_ == OfferType::SELL ? *this : other;
    size_t deal_amount = std::min(buy_offer.amount_, sell_offer.amount_);
    int deal_price =
        this->type_ == OfferType::SELL ? buy_offer.price_ : sell_offer.price_;

    buy_offer.amount_ -= deal_amount;
    sell_offer.amount_ -= deal_amount;

    if (buy_offer.amount_ == 0) {
        buy_offer.status_ = OfferStatus::FULLFILLED;
    }
    if (sell_offer.amount_ == 0) {
        sell_offer.status_ = OfferStatus::FULLFILLED;
    }

    return Deal(sell_offer.owner_id_, buy_offer.owner_id_, deal_price,
                deal_amount);
}

int Offer::GetPrice() const { return price_; }

size_t Offer::GetAmount() const { return amount_; }

uint64_t Offer::GetId() const { return id_; }

uint64_t Offer::GetOwnerId() const { return owner_id_; }

OfferType Offer::GetType() const { return type_; }

OfferStatus Offer::GetStatus() const { return status_; }

uint64_t Offer::GenerateId() { return offer_id_++; }

bool operator<(const std::shared_ptr<Offer>& lhs,
               const std::shared_ptr<Offer>& rhs) {
    return lhs->GetId() < rhs->GetId();
}

bool operator<(uint64_t lhs, const std::shared_ptr<Offer>& rhs) {
    return lhs < rhs->GetId();
}

bool operator<(const std::shared_ptr<Offer>& lhs, uint64_t rhs) {
    return lhs->GetId() < rhs;
}
