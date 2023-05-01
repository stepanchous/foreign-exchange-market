#pragma once

#include <string>

static short port = 5555;

// Request types
namespace requests {

static inline const std::string REGISTRATION = "Reg";
static inline const std::string BALANCE = "Balance";
static inline const std::string ACTIVE_OFFERS = "Active";
static inline const std::string CLOSED_DEALS = "Deal";
static inline const std::string REG_CONFIRMATION = "RegConf";
static inline const std::string POST_OFFER = "PostOffer";
static inline const std::string QUOTES = "Quotes";
static inline const std::string CANCEL = "Cancel";
static inline const std::string LOGIN = "Log";

}  // namespace requests
// namespace requests

// Fields of json that is sent to sockets
namespace json_field {

static inline const std::string TYPE = "TYPE";
static inline const std::string USER_ID = "USER_ID";
static inline const std::string OFFER_ID = "OFFER_ID";
static inline const std::string DEAL_ID = "DEAL_ID";
static inline const std::string BUY = "BUY";
static inline const std::string SELL = "SELL";
static inline const std::string BUY_SELL = "BUY-SELL";
static inline const std::string PRICE = "PRICE";
static inline const std::string AMOUNT = "AMOUNT";
static inline const std::string SUCCESS = "SUCCESS";
static inline const std::string USD = "USD";
static inline const std::string RUB = "RUB";
static inline const std::string USERNAME = "USERNAME";
static inline const std::string OFFER_SIDE = "OFFER_SIDE";
static inline const std::string QUOTE = "QUOTE";
static inline const std::string ASK_QUOTE = "ASK_QUOTE";
static inline const std::string BID_QUOTE = "BID_QUOTE";
static inline const std::string SPREAD = "SPREAD";
static inline const std::string PW_HASH = "PW_HASH";

}  // namespace json_field
