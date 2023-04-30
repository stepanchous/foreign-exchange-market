#pragma once

#include <stddef.h>

#include <atomic>
#include <cstdint>
#include <memory>

#include "deal.h"

enum class OfferType {
    BUY,
    SELL,
};

enum class OfferStatus {
    ACTIVE,
    FULLFILLED,
};

class Offer {
   public:
    explicit Offer(uint64_t owner_id, OfferType type, int price, size_t amount);

    Deal MakeDeal(Offer& other);

    int GetPrice() const;

    size_t GetAmount() const;

    uint64_t GetId() const;

    uint64_t GetOwnerId() const;

    OfferType GetType() const;

    OfferStatus GetStatus() const;

   private:
    static uint64_t GenerateId();

   private:
    uint64_t id_;
    uint64_t owner_id_;
    OfferType type_;
    int price_;
    size_t amount_;
    OfferStatus status_;

    static std::atomic<uint64_t> offer_id_;
};

bool operator<(const std::shared_ptr<Offer>& lhs,
               const std::shared_ptr<Offer>& rhs);

bool operator<(uint64_t lhs, const std::shared_ptr<Offer>& rhs);

bool operator<(const std::shared_ptr<Offer>& lhs, uint64_t rhs);
