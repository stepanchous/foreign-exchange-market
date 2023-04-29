#include <optional>
#define CATCH_CONFIG_MAIN

#include <boost/uuid/uuid.hpp>
#include <catch2/catch.hpp>
#include <set>

#include "../src/market.h"

using namespace std;

TEST_CASE("Register User", "[market]") {
    Market market;

    const auto user_id1 = market.RegisterUser("user1");
    const auto user_id2 = market.RegisterUser("user2");
    REQUIRE(user_id1 != user_id2);

    REQUIRE(market.GetUserBalance(user_id1).usd == 0);
    REQUIRE(market.GetUserBalance(user_id1).rub == 0);

    set<shared_ptr<Offer>, less<>> expected_active_offers = {};
    REQUIRE(market.GetActiveOffers(user_id1) == expected_active_offers);

    set<shared_ptr<Deal>, less<>> expected_close_deals = {};
    REQUIRE(market.GetClosedDeals(user_id1) == expected_close_deals);
}

TEST_CASE("Deposit currency", "[market]") {
    Market market;
    const auto user_id = market.RegisterUser("user1");

    market.DepositUSD(user_id, 100);
    market.DepositRUB(user_id, 120);

    Balance expected_balance1 = {.usd = 100, .rub = 120};
    REQUIRE(market.GetUserBalance(user_id) == expected_balance1);

    market.DepositUSD(user_id, 50);

    Balance expected_balance2 = {.usd = 150, .rub = 120};
    REQUIRE(market.GetUserBalance(user_id) == expected_balance2);
}

TEST_CASE("Post perfectly matching sell-buy offers", "[market]") {
    Market market;
    const auto user_id1 = market.RegisterUser("user1");
    const auto user_id2 = market.RegisterUser("user2");

    {
        auto offer_type = OfferType::SELL;
        int price = 80;
        size_t amount = 10;
        market.PostOffer(user_id1, offer_type, price, amount);
    }

    Balance expected_balance = {.usd = 0, .rub = 0};
    REQUIRE(market.GetUserBalance(user_id1) == expected_balance);
    REQUIRE(market.GetActiveOffers(user_id1).size() == 1);
    REQUIRE(market.GetClosedDeals(user_id1).empty() == true);

    {
        auto offer_type = OfferType::BUY;
        int price = 80;
        size_t amount = 10;
        market.PostOffer(user_id2, offer_type, price, amount);
    }

    Balance expected_balance_buyer = {.usd = 10, .rub = -800};
    Balance expected_balance_seller = {.usd = -10, .rub = 800};

    REQUIRE(market.GetUserBalance(user_id1) == expected_balance_seller);
    REQUIRE(market.GetUserBalance(user_id2) == expected_balance_buyer);
    REQUIRE(market.GetActiveOffers(user_id2).empty());
    REQUIRE(market.GetActiveOffers(user_id1).empty());
    REQUIRE(market.GetClosedDeals(user_id1).begin()->get()->GetId() ==
            market.GetClosedDeals(user_id2).begin()->get()->GetId());
}

TEST_CASE("Fullfill buy offer with two sell offers") {
    Market market;
    const auto user_id1 = market.RegisterUser("user1");
    const auto user_id2 = market.RegisterUser("user2");

    {
        auto offer_type = OfferType::BUY;
        int price1 = 70;
        int price2 = 80;
        size_t amount1 = 15;
        size_t amount2 = 5;
        market.PostOffer(user_id1, offer_type, price1, amount1);
        market.PostOffer(user_id1, offer_type, price2, amount2);
    }
    {
        auto offer_type = OfferType::SELL;
        int price = 60;
        size_t amount = 20;
        market.PostOffer(user_id2, offer_type, price, amount);
    }

    Balance expected_balance_buyer = {.usd = 20, .rub = -1450};
    Balance expected_balance_seller = {.usd = -20, .rub = 1450};

    REQUIRE(market.GetUserBalance(user_id1) == expected_balance_buyer);
    REQUIRE(market.GetUserBalance(user_id2) == expected_balance_seller);
    REQUIRE(market.GetActiveOffers(user_id1).empty());
    REQUIRE(market.GetActiveOffers(user_id2).empty());
    REQUIRE(market.GetClosedDeals(user_id1).size() == 2);
    REQUIRE(market.GetClosedDeals(user_id2).size() == 2);
}

TEST_CASE("Partial fullfillment of buy request with prioritization") {
    Market market;
    const auto user_id1 = market.RegisterUser("user1");
    const auto user_id2 = market.RegisterUser("user2");
    const auto user_id3 = market.RegisterUser("user3");
    {
        auto offer_type = OfferType::SELL;
        int price = 61;
        size_t amount = 10;
        market.PostOffer(user_id1, offer_type, price, amount);
    }
    {
        auto offer_type = OfferType::SELL;
        int price = 62;
        size_t amount = 10;
        market.PostOffer(user_id2, offer_type, price, amount);
    }
    {
        auto offer_type = OfferType::BUY;
        int price = 64;
        size_t amount = 30;
        market.PostOffer(user_id3, offer_type, price, amount);
    }

    Balance expected_balance_user1 = {.usd = -10, .rub = 610};
    Balance expected_balance_user2 = {.usd = -10, .rub = 620};
    Balance expected_balance_user3 = {.usd = 20, .rub = -1230};

    REQUIRE(market.GetUserBalance(user_id1) == expected_balance_user1);
    REQUIRE(market.GetUserBalance(user_id2) == expected_balance_user2);
    REQUIRE(market.GetUserBalance(user_id3) == expected_balance_user3);

    REQUIRE(market.GetClosedDeals(user_id1).size() == 1);
    REQUIRE(market.GetClosedDeals(user_id2).size() == 1);
    REQUIRE(market.GetClosedDeals(user_id3).size() == 2);

    REQUIRE(market.GetActiveOffers(user_id1).empty());
    REQUIRE(market.GetActiveOffers(user_id2).empty());
    REQUIRE(market.GetActiveOffers(user_id3).size() == 1);

    SECTION("Fullfill partial offer") {
        auto user_id4 = market.RegisterUser("user4");
        {
            auto offer_type = OfferType::SELL;
            int price = 60;
            size_t amount = 10;
            market.PostOffer(user_id4, offer_type, price, amount);
        }

        expected_balance_user3 = {.usd = 30, .rub = -1870};
        Balance expected_balance_user4 = {.usd = -10, .rub = 640};

        REQUIRE(market.GetUserBalance(user_id3) == expected_balance_user3);
        REQUIRE(market.GetUserBalance(user_id4) == expected_balance_user4);

        REQUIRE(market.GetClosedDeals(user_id3).size() == 3);
        REQUIRE(market.GetClosedDeals(user_id4).size() == 1);

        REQUIRE(market.GetActiveOffers(user_id3).empty());
        REQUIRE(market.GetActiveOffers(user_id3).empty());
    }
}

TEST_CASE("Task example") {
    Market market;
    const auto user_id1 = market.RegisterUser("user1");
    const auto user_id2 = market.RegisterUser("user2");
    const auto user_id3 = market.RegisterUser("user3");
    {
        auto offer_type = OfferType::BUY;
        int price = 62;
        size_t amount = 10;
        market.PostOffer(user_id1, offer_type, price, amount);
    }
    {
        auto offer_type = OfferType::BUY;
        int price = 63;
        size_t amount = 20;
        market.PostOffer(user_id2, offer_type, price, amount);
    }
    {
        auto offer_type = OfferType::SELL;
        int price = 61;
        size_t amount = 50;
        market.PostOffer(user_id3, offer_type, price, amount);
    }

    Balance expected_balance_user1 = {.usd = 10, .rub = -620};
    Balance expected_balance_user2 = {.usd = 20, .rub = -1260};
    Balance expected_balance_user3 = {.usd = -30, .rub = 1880};

    REQUIRE(market.GetUserBalance(user_id1) == expected_balance_user1);
    REQUIRE(market.GetUserBalance(user_id2) == expected_balance_user2);
    REQUIRE(market.GetUserBalance(user_id3) == expected_balance_user3);

    REQUIRE(market.GetClosedDeals(user_id1).size() == 1);
    REQUIRE(market.GetClosedDeals(user_id2).size() == 1);
    REQUIRE(market.GetClosedDeals(user_id3).size() == 2);

    REQUIRE(market.GetActiveOffers(user_id1).empty());
    REQUIRE(market.GetActiveOffers(user_id2).empty());
    REQUIRE(market.GetActiveOffers(user_id3).size() == 1);
}

TEST_CASE("Get quotes") {
    SECTION("No offers and deals") {
        Market market;
        std::optional<int> expected_quote = std::nullopt;
        AskBidQuotesInfo expected_ask_bid_quotes = {.ask_quote = std::nullopt,
                                                    .bid_quote = std::nullopt,
                                                    .spread = std::nullopt};
        REQUIRE(market.GetQuote() == expected_quote);
        REQUIRE(market.GetAskBidQuotes() == expected_ask_bid_quotes);
    }

    SECTION("Ask quote") {
        Market market;
        auto user_id1 = market.RegisterUser("user1");

        OfferType offer_type = OfferType::BUY;
        int price1 = 60;
        int price2 = 70;
        size_t amount = 20;

        market.PostOffer(user_id1, offer_type, price1, amount);
        market.PostOffer(user_id1, offer_type, price2, amount);

        AskBidQuotesInfo expected_ask_bid_quotes = {.ask_quote = price1,
                                                    .bid_quote = std::nullopt,
                                                    .spread = std::nullopt};

        REQUIRE(market.GetAskBidQuotes() == expected_ask_bid_quotes);
    }

    SECTION("Bid quote") {
        Market market;
        auto user_id1 = market.RegisterUser("user1");

        OfferType offer_type = OfferType::SELL;
        int price1 = 60;
        int price2 = 70;
        size_t amount = 20;

        market.PostOffer(user_id1, offer_type, price1, amount);
        market.PostOffer(user_id1, offer_type, price2, amount);

        AskBidQuotesInfo expected_ask_bid_quotes = {.ask_quote = std::nullopt,
                                                    .bid_quote = price2,
                                                    .spread = std::nullopt};

        REQUIRE(market.GetAskBidQuotes() == expected_ask_bid_quotes);
    }

    SECTION("All quotes and spread") {
        Market market;
        auto user_id1 = market.RegisterUser("user1");
        auto user_id2 = market.RegisterUser("user2");

        {
            OfferType offer_type = OfferType::SELL;
            int price1 = 61;
            int price2 = 62;
            int price3 = 63;
            size_t amount = 10;

            market.PostOffer(user_id1, offer_type, price1, amount);
            market.PostOffer(user_id1, offer_type, price2, amount);
            market.PostOffer(user_id1, offer_type, price3, amount);
        }
        {
            OfferType offer_type = OfferType::BUY;
            int price = 62;
            size_t amount = 30;

            market.PostOffer(user_id2, offer_type, price, amount);
        }
        std::optional<int> expected_quote = 62;
        AskBidQuotesInfo expected_ask_bid_quotes = {
            .ask_quote = 62, .bid_quote = 63, .spread = 1};

        REQUIRE(market.GetQuote() == expected_quote);
        REQUIRE(market.GetAskBidQuotes() == expected_ask_bid_quotes);
    }
}

TEST_CASE("Offer rejection") {
    SECTION("Post and decline one offer") {
        Market market;
        auto user_id = market.RegisterUser("user");
        {
            OfferType offer_type = OfferType::BUY;
            int price = 80;
            size_t amount = 10;

            auto offer_id =
                market.PostOffer(user_id, offer_type, price, amount);
            market.RemoveOffer(user_id, offer_id);

            set<shared_ptr<Offer>, std::less<>> expected_active_offers = {};
            set<shared_ptr<Deal>, std::less<>> expected_closed_deals = {};

            std::optional<int> expected_quote = std::nullopt;
            AskBidQuotesInfo expected_ask_bid_quotes = {
                .ask_quote = std::nullopt,
                .bid_quote = std::nullopt,
                .spread = std::nullopt};

            REQUIRE(market.GetActiveOffers(user_id) == expected_active_offers);
            REQUIRE(market.GetClosedDeals(user_id) == expected_closed_deals);
            REQUIRE(market.GetQuote() == expected_quote);
            REQUIRE(market.GetAskBidQuotes() == expected_ask_bid_quotes);
        }
    }
    SECTION("Two matching offers with one rejected") {
        Market market;
        auto user_id1 = market.RegisterUser("user1");
        auto user_id2 = market.RegisterUser("user2");
        {
            OfferType offer_type = OfferType::BUY;
            int price = 80;
            size_t amount = 10;

            auto offer_id =
                market.PostOffer(user_id1, offer_type, price, amount);
            market.RemoveOffer(user_id1, offer_id);
        }
        {
            OfferType offer_type = OfferType::SELL;
            int price = 80;
            size_t amount = 10;

            market.PostOffer(user_id2, offer_type, price, amount);
        }
        set<shared_ptr<Offer>, std::less<>> expected_active_offers1 = {};
        size_t expected_active_offers_size2 = 1;
        set<shared_ptr<Deal>, std::less<>> expected_closed_deals1 = {};
        set<shared_ptr<Deal>, std::less<>> expected_closed_deals2 = {};

        std::optional<int> expected_quote = std::nullopt;
        AskBidQuotesInfo expected_ask_bid_quotes = {
            .ask_quote = std::nullopt, .bid_quote = 80, .spread = std::nullopt};

        REQUIRE(market.GetActiveOffers(user_id1) == expected_active_offers1);
        REQUIRE(market.GetActiveOffers(user_id2).size() ==
                expected_active_offers_size2);
        REQUIRE(market.GetClosedDeals(user_id1) == expected_closed_deals1);
        REQUIRE(market.GetClosedDeals(user_id2) == expected_closed_deals2);
        REQUIRE(market.GetQuote() == expected_quote);
        REQUIRE(market.GetAskBidQuotes() == expected_ask_bid_quotes);
    }

    SECTION("Skip declined offer") {
        Market market;
        auto user_id1 = market.RegisterUser("user1");
        auto user_id2 = market.RegisterUser("user2");

        {
            OfferType offer_type = OfferType::SELL;
            int price1 = 61;
            int price2 = 62;
            int price3 = 63;
            size_t amount = 10;

            market.PostOffer(user_id1, offer_type, price1, amount);
            auto offer_id =
                market.PostOffer(user_id1, offer_type, price1, amount);
            market.PostOffer(user_id1, offer_type, price2, amount);
            market.PostOffer(user_id1, offer_type, price3, amount);

            market.RemoveOffer(user_id1, offer_id);
        }
        {
            OfferType offer_type = OfferType::BUY;
            int price = 62;
            size_t amount = 30;

            market.PostOffer(user_id2, offer_type, price, amount);
        }
        Balance expected_balance1 = {.usd = -20, .rub = 1230};
        Balance expected_balance2 = {.usd = 20, .rub = -1230};

        std::optional<int> expected_quote = 62;
        AskBidQuotesInfo expected_ask_bid_quotes = {
            .ask_quote = 62, .bid_quote = 63, .spread = 1};

        REQUIRE(market.GetUserBalance(user_id1) == expected_balance1);
        REQUIRE(market.GetUserBalance(user_id2) == expected_balance2);
        REQUIRE(market.GetActiveOffers(user_id1).size() == 1);
        REQUIRE(market.GetActiveOffers(user_id2).size() == 1);
        REQUIRE(market.GetClosedDeals(user_id1).size() == 2);
        REQUIRE(market.GetClosedDeals(user_id2).size() == 2);
        REQUIRE(market.GetQuote() == expected_quote);
        REQUIRE(market.GetAskBidQuotes() == expected_ask_bid_quotes);
    }
}
