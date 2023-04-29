#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <string>

#include "offer.h"

struct Balance {
    int usd;
    int rub;

    bool operator<=>(const Balance&) const = default;
};

class UserData {
   public:
    UserData(const std::string& username);

    void AddOffer(const std::shared_ptr<Offer>& offer);

    void AddDeal(const std::shared_ptr<Deal>& deal);

    bool RemoveActiveOffer(uint64_t offer_id);

    Balance GetBalance() const;

    const std::set<std::shared_ptr<Offer>, std::less<>>& GetActiveOffers()
        const;

    const std::set<std::shared_ptr<Deal>, std::less<>>& GetClosedDeals() const;

    void DepositUSD(size_t deposit_amount);

    void DepositRUB(size_t deposit_amount);

    void WithdrawUSD(size_t withdraw_amount);

    void WithdrawRUB(size_t withdraw_amount);

   private:
    std::string username_;
    Balance balance_;
    std::set<std::shared_ptr<Offer>, std::less<>> active_offers_;
    std::set<std::shared_ptr<Deal>, std::less<>> closed_deals_;
};
