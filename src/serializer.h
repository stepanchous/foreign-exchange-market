#pragma once

#include <cstdint>
#include <string>

#include "market.h"
#include "offer.h"

class Serializer {
   public:
    std::string RegisterUser(const std::string& username, size_t pw_hash);

    std::string Login(const std::string& username, size_t pw_hash);

    std::string GetActiveOffers(uint64_t user_id) const;

    std::string GetClosedDeals(uint64_t user_id) const;

    std::string GetBalance(uint64_t user_id) const;

    std::string GetQuotes();

    void PostOffer(uint64_t user_id, OfferType offer_type, int price,
                   size_t amount);

    std::string CancelOffer(uint64_t user_id, uint64_t offer_id);

   private:
    static std::string OfferTypeToString(OfferType offer_type);

   private:
    Market market_;
};

Serializer& GetSerializer();
