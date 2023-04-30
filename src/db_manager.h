#pragma once

#include <sqlite3.h>

#include <cstdint>
#include <ostream>
#include <string>

#include "logger.h"
#include "offer.h"

class DBManager {
   public:
    DBManager(std::ostream& log_output);

    ~DBManager();

    void AddOffer(uint64_t offer_id, uint64_t owner_id, OfferType offer_type,
                  size_t amount, int price);

    void AddDeal(uint64_t deal_id, uint64_t seller_id, uint64_t buyer_id,
                 size_t amount, int price);

    int GetMaxId(const std::string& table);

   private:
    static int GetMaxIdCallback(void* id, int count, char** data,
                                char** columns);

   private:
    sqlite3* db_;
    Logger logger_;

    static inline const std::string db_path = "db/market.db";
};

DBManager& GetDBManager();
