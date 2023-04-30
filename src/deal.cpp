#include "deal.h"

#include <cstdint>

#include "db_manager.h"

Deal::Deal(uint64_t seller_id, uint64_t buyer_id, int price, size_t amount)
    : id_(GenerateId()),
      seller_id_(seller_id),
      buyer_id_(buyer_id),
      price_(price),
      amount_(amount) {
    GetDBManager().AddDeal(id_, seller_id_, buyer_id_, amount, price);
}

uint64_t Deal::GetId() const { return id_; }

uint64_t Deal::GetSeller() const { return seller_id_; }

uint64_t Deal::GetBuyer() const { return buyer_id_; }

int Deal::GetPrice() const { return price_; }

size_t Deal::GetAmount() const { return amount_; }

uint64_t Deal::GenerateId() { return deal_id_++; }

bool operator<(const std::shared_ptr<Deal>& lhs,
               const std::shared_ptr<Deal>& rhs) {
    return lhs->GetId() < rhs->GetId();
}
