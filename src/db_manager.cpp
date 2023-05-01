#include "db_manager.h"

#include <cstdint>
#include <format>
#include <iostream>
#include <optional>
#include <stdexcept>

#include "logger.h"
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
        "  PW_HASH   INT NOT NULL"
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

void DBManager::AddUser(uint64_t user_id, const std::string& username,
                        size_t pw_hash) {
    std::string query = std::format("INSERT INTO User VALUES({}, \"{}\", {});",
                                    user_id, username, pw_hash);
    int db_error = sqlite3_exec(db_, query.c_str(), NULL, NULL, NULL);
    if (db_error) {
        logger_.Log(
            LogType::WARNING,
            std::format("Failed to add user with id {} to db", user_id));
    } else {
        logger_.Log(LogType::INFO,
                    std::format("User with id {} was added to db", user_id));
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

int DBManager::UsernameExistCallback(void* exist, int, char** data, char**) {
    int* exist_ = (int*)exist;
    *exist_ = std::stoi(data[0]);

    return 0;
}

bool DBManager::UsernameExist(const std::string& username) {
    std::string query = std::format(
        "SELECT EXISTS(SELECT 1 FROM User WHERE USERNAME=\"{}\" LIMIT 1);",
        username);

    int exist;
    int db_error =
        sqlite3_exec(db_, query.c_str(), UsernameExistCallback, &exist, NULL);
    if (db_error) {
        logger_.Log(LogType::WARNING, "Could not find out if username exist");
    } else {
        logger_.Log(LogType::INFO, "Found out if username exist");
    }

    return exist;
}

int DBManager::GetUserIdCallback(void* id, int, char** data, char**) {
    if (data == nullptr || data[0] == nullptr) {
        return 0;
    }
    int* user_id = (int*)id;
    *user_id = std::stoi(data[0]);

    return 0;
}

std::optional<uint64_t> DBManager::GetUserId(const std::string& username,
                                             size_t pw_hash) {
    std::string query = std::format(
        "SELECT * FROM User WHERE (USERNAME = \"{}\" AND PW_HASH = {}) LIMIT "
        "1;",
        username, pw_hash);

    int user_id = -1;
    int db_error =
        sqlite3_exec(db_, query.c_str(), GetUserIdCallback, &user_id, NULL);
    if (db_error) {
        logger_.Log(LogType::WARNING,
                    "Unable to find user with given username and password");
    } else {
        logger_.Log(LogType::INFO,
                    "User with given username and password was found");
    }

    return user_id == -1 ? std::nullopt : std::optional<uint64_t>(user_id);
}

DBManager::~DBManager() { sqlite3_close(db_); }

DBManager& GetDBManager() {
    static DBManager db_manager(std::cout);
    return db_manager;
}
