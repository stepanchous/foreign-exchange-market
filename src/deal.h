#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

class Deal {
   public:
    Deal(uint64_t seller_id, uint64_t buyer_id_, int price, size_t amount);

    uint64_t GetId() const;

    uint64_t GetSeller() const;

    uint64_t GetBuyer() const;

    int GetPrice() const;

    size_t GetAmount() const;

   private:
    static uint64_t GenerateId();

   private:
    uint64_t id_;
    uint64_t seller_id_;
    uint64_t buyer_id_;
    int price_;
    size_t amount_;

    static inline std::atomic<uint64_t> deal_id_ = 0;
};

bool operator<(const std::shared_ptr<Deal>& lhs,
               const std::shared_ptr<Deal>& rhs);
