#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "offer.h"
#include "user_data.h"

struct OfferQueue {
    int price;
    mutable std::deque<std::weak_ptr<Offer>> offers;
};

struct AskBidQuotesInfo {
    std::optional<int> ask_quote;
    std::optional<int> bid_quote;
    std::optional<int> spread;

    bool operator<=>(const AskBidQuotesInfo& other) const = default;
};

bool operator<(const OfferQueue& lhs, const OfferQueue& rhs);
bool operator<(int lhs, const OfferQueue& rhs);
bool operator<(const OfferQueue& lhs, int rhs);

class Market {
   public:
    std::optional<uint64_t> RegisterUser(const std::string& username,
                                         size_t pw_hash);

    void DepositUSD(uint64_t user_id, size_t usd_amount);

    void DepositRUB(uint64_t user_id, size_t rub_amount);

    Balance GetUserBalance(uint64_t user_id) const;

    const std::set<std::shared_ptr<Offer>, std::less<>>& GetActiveOffers(
        uint64_t user_id) const;

    const std::set<std::shared_ptr<Deal>, std::less<>>& GetClosedDeals(
        uint64_t user_id) const;

    uint64_t PostOffer(uint64_t user_id, OfferType offer_type, int price,
                       size_t amount);

    bool RemoveOffer(uint64_t user_id, uint64_t offer_id);

    std::optional<int> GetQuote() const;

    AskBidQuotesInfo GetAskBidQuotes();

   private:
    void RegisterDeal(const std::shared_ptr<Deal>& deal);

    void UpdateQuote(const std::shared_ptr<Deal>& deal);

    template <typename BorderPricePredicate, typename GetBestOffer>
    void ProcessOffer(const std::shared_ptr<Offer>& offer,
                      BorderPricePredicate border_price_predicate,
                      GetBestOffer get_best_offer);

    void AddActiveOffer(const std::shared_ptr<Offer>& offer);

    std::optional<int> DetermineQuote(OfferType offer_type);

   private:
    std::unordered_map<uint64_t, UserData> user_id_to_user_data_;

    std::set<OfferQueue, std::less<>> active_sell_offers_;
    std::set<OfferQueue, std::less<>> active_buy_offers_;
    std::optional<int> quote_;
};

template <typename BorderPricePredicate, typename GetBestOffers>
void Market::ProcessOffer(const std::shared_ptr<Offer>& offer,
                          BorderPricePredicate border_price_predicate,
                          GetBestOffers get_best_offers) {
    auto& offers = offer->GetType() == OfferType::SELL ? active_buy_offers_
                                                       : active_sell_offers_;
    while (offer->GetStatus() != OfferStatus::FULLFILLED) {
        if (offers.empty() ||
            !border_price_predicate(offers, offer->GetPrice())) {
            AddActiveOffer(offer);
            break;
        }

        auto best_offers = get_best_offers(offers);
        std::weak_ptr<Offer>& best_offer = best_offers->offers.back();
        if (best_offer.expired()) {
            best_offers->offers.pop_back();
            if (best_offers->offers.empty()) {
                offers.erase(best_offers);
            }
            continue;
        }

        auto deal = std::make_shared<Deal>(offer->MakeDeal(*best_offer.lock()));
        RegisterDeal(deal);
        UpdateQuote(deal);
        if (best_offer.lock()->GetStatus() == OfferStatus::FULLFILLED) {
            best_offers->offers.pop_back();
            user_id_to_user_data_.at(best_offer.lock()->GetOwnerId())
                .RemoveActiveOffer(best_offer.lock()->GetId());
            if (best_offers->offers.empty()) {
                offers.erase(best_offers);
            }
        }
    }

    if (offer->GetStatus() == OfferStatus::FULLFILLED) {
        user_id_to_user_data_.at(offer->GetOwnerId())
            .RemoveActiveOffer(offer->GetId());
    }
}
