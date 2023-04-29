#pragma once

#include <cstdint>
#include <string>

#include "market.h"
#include "offer.h"

static inline const std::string BUY_STR = "BUY";
static inline const std::string SELL_STR = "SELL";
static inline const std::string ID_FIELD = "ID";
static inline const std::string PRICE_FIELD = "PRICE";
static inline const std::string AMOUNT_FIELD = "AMOUNT";
static inline const std::string USD_STR = "USD";
static inline const std::string RUB_STR = "RUB";

class Serializer {
   public:
    std::string RegisterUser(const std::string& username);

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
