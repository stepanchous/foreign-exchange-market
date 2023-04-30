#include "user_data.h"

#include <cstdint>
#include <memory>
#include <string>

#ifndef TEST
#include "db_manager.h"
#endif  // !TEST

UserData::UserData(const std::string& username, size_t pw_hash)
    : id_(GenerateId()), username_(username), balance_({.usd = 0, .rub = 0}) {
#ifndef TEST
    GetDBManager().AddUser(id_, username_, pw_hash);
#endif  // !TEST
}

void UserData::AddOffer(const std::shared_ptr<Offer>& offer) {
    active_offers_.insert(offer);
}

void UserData::AddDeal(const std::shared_ptr<Deal>& deal) {
    closed_deals_.insert(deal);
}

bool UserData::RemoveActiveOffer(uint64_t offer_id) {
    auto offer = active_offers_.find(offer_id);
    if (offer == active_offers_.end()) {
        return false;
    }
    active_offers_.erase(offer);

    return true;
}

uint64_t UserData::GetId() const { return id_; }

Balance UserData::GetBalance() const { return balance_; }

const std::set<std::shared_ptr<Offer>, std::less<>>& UserData::GetActiveOffers()
    const {
    return active_offers_;
}

const std::set<std::shared_ptr<Deal>, std::less<>>& UserData::GetClosedDeals()
    const {
    return closed_deals_;
}

void UserData::DepositUSD(size_t deposit_amount) {
    balance_.usd += deposit_amount;
}

void UserData::DepositRUB(size_t deposit_amount) {
    balance_.rub += deposit_amount;
}

void UserData::WithdrawUSD(size_t withdraw_amount) {
    balance_.usd -= withdraw_amount;
}

void UserData::WithdrawRUB(size_t withdraw_amount) {
    balance_.rub -= withdraw_amount;
}

uint64_t UserData::GenerateId() { return user_id_++; }

size_t UserDataHasher::operator()(const UserData& user_data) {
    return hash_type{}(user_data.GetId());
}

size_t UserDataHasher::operator()(uint64_t user_id) {
    return hash_type{}(user_id);
}

std::optional<UserData> CreateUser(const std::string& username,
                                   size_t pw_hash) {
#ifndef TEST
    if (GetDBManager().UsernameExist(username)) {
        return std::nullopt;
    }
#endif  // !TEST

    return UserData(username, pw_hash);
}
