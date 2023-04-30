#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
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
    UserData(const std::string& username, size_t pw_hash);

    void AddOffer(const std::shared_ptr<Offer>& offer);

    void AddDeal(const std::shared_ptr<Deal>& deal);

    bool RemoveActiveOffer(uint64_t offer_id);

    uint64_t GetId() const;

    Balance GetBalance() const;

    const std::set<std::shared_ptr<Offer>, std::less<>>& GetActiveOffers()
        const;

    const std::set<std::shared_ptr<Deal>, std::less<>>& GetClosedDeals() const;

    void DepositUSD(size_t deposit_amount);

    void DepositRUB(size_t deposit_amount);

    void WithdrawUSD(size_t withdraw_amount);

    void WithdrawRUB(size_t withdraw_amount);

   private:
    static uint64_t GenerateId();

   private:
    uint64_t id_;
    std::string username_;
    Balance balance_;
    std::set<std::shared_ptr<Offer>, std::less<>> active_offers_;
    std::set<std::shared_ptr<Deal>, std::less<>> closed_deals_;

    static std::atomic<uint64_t> user_id_;
};

struct UserDataHasher {
    using hash_type = std::hash<uint64_t>;
    using is_transparent = void;

    std::size_t operator()(const UserData& user_data);
    std::size_t operator()(uint64_t id);
};

std::optional<UserData> CreateUser(const std::string& username, size_t pw_hash);
