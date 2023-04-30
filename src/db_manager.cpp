#include "db_manager.h"

#include <format>
#include <iostream>
#include <stdexcept>

#include "offer.h"

DBManager::DBManager(std::ostream& log_output) : logger_(log_output) {
    int db_error;

    db_error = sqlite3_open(db_path.c_str(), &db_);
    if (db_error) {
        logger_.Log(LogType::ERROR, "DB connection failed");
        throw std::runtime_error("Unable to connect to db.");
    }

    std::string create_user_query =
        "  CREATE TABLE IF NOT EXISTS User("
        "  ID        INT PRIMARY KEY NOT NULL,"
        "  USERNAME  TEXT NOT NULL,"
        "  PW_HASH   VARCHAR(64) NOT NULL,"
        "  USD       INT NOT NULL,"
        "  RUB       INT NOT NULL"
        ");";

    std::string create_deal_query =
        "CREATE TABLE IF NOT EXISTS Deal ("
        "  ID         INT PRIMARY KEY NOT NULL,"
        "  SELLER_ID  INT NOT NULL,"
        "  BUYER_ID   INT NOT NULL,"
        "  AMOUNT     INT NOT NULL,"
        "  PRICE      INT NOT NULL"
        ");";

    std::string create_offer_query =
        "CREATE TABLE IF NOT EXISTS Offer ("
        "  ID         INT PRIMARY KEY NOT NULL,"
        "  OWNER_ID   INT NOT NULL,"
        "  TYPE       BOOL NOT NULL,"
        "  AMOUNT     INT NOT NULL,"
        "  PRICE      INT NOT NULL"
        ");";

    sqlite3_exec(db_, create_user_query.c_str(), NULL, NULL, NULL);
    sqlite3_exec(db_, create_deal_query.c_str(), NULL, NULL, NULL);
    sqlite3_exec(db_, create_offer_query.c_str(), NULL, NULL, NULL);

    logger_.Log(LogType::INFO, "DB connection established");
}

void DBManager::AddOffer(uint64_t offer_id, uint64_t owner_id,
                         OfferType offer_type, size_t amount, int price) {
    std::string query = std::format(
        "INSERT INTO Offer VALUES({}, {}, {}, {}, {});", offer_id, owner_id,
        static_cast<int>(offer_type == OfferType::BUY), amount, price);
    int db_error = sqlite3_exec(db_, query.c_str(), NULL, NULL, NULL);
    if (db_error) {
        logger_.Log(
            LogType::WARNING,
            std::format("Failed to add offer with id {} to db", offer_id));
    } else {
        logger_.Log(LogType::INFO,
                    std::format("Offer with id {} was added to db", offer_id));
    }
}

void DBManager::AddDeal(uint64_t deal_id, uint64_t seller_id, uint64_t buyer_id,
                        size_t amount, int price) {
    std::string query =
        std::format("INSERT INTO Deal VALUES({}, {}, {}, {}, {})", deal_id,
                    seller_id, buyer_id, amount, price);
    int db_error = sqlite3_exec(db_, query.c_str(), NULL, NULL, NULL);
    if (db_error) {
        logger_.Log(
            LogType::WARNING,
            std::format("Failed to add deal with id {} to db", deal_id));
    } else {
        logger_.Log(LogType::INFO,
                    std::format("Deal with id {} was added to db", deal_id));
    }
}

int DBManager::GetMaxId(const std::string& table) {
    int id = -1;
    std::string query = std::format("SELECT MAX(ID) FROM {};", table);
    int db_error =
        sqlite3_exec(db_, query.c_str(), GetMaxIdCallback, &id, NULL);
    if (db_error) {
        logger_.Log(LogType::ERROR,
                    std::format("Unable to init id for table {}", table));
    } else {
        logger_.Log(LogType::INFO,
                    std::format("For table {} init id is {}", table, id + 1));
    }

    return id + 1;
}

int DBManager::GetMaxIdCallback(void* id, int, char** data, char**) {
    if (data == nullptr || data[0] == nullptr) {
        return 0;
    }

    int* max_id = (int*)id;
    *max_id = std::stoi(data[0]);

    return 0;
}

DBManager::~DBManager() { sqlite3_close(db_); }

DBManager& GetDBManager() {
    static DBManager db_manager(std::cout);
    return db_manager;
}
