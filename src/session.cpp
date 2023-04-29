#include "session.h"

#include <boost/asio/buffer.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/write.hpp>
#include <string>

#include "common.h"
#include "json.h"
#include "offer.h"
#include "serializer.h"

using namespace boost::asio;
using nlohmann::json;

Session::Session(io_service& io_service) : socket_(io_service) {}

void Session::Start() {
    socket_.async_read_some(
        buffer(data_, max_length),
        boost::bind(&Session::HandleRead, this, placeholders::error,
                    placeholders::bytes_transferred));
}

void Session::HandleRead(const boost::system::error_code& error,
                         size_t bytes_transferred) {
    if (!error) {
        data_[bytes_transferred] = '\0';

        json request = json::parse(data_);
        auto request_type = request[json_field::TYPE];
        std::string reply;
        if (request_type == requests::REGISTRATION) {
            reply = GetSerializer().RegisterUser(request[json_field::USERNAME]);
        } else if (request_type == requests::BALANCE) {
            reply = GetSerializer().GetBalance(request[json_field::USER_ID]);
        } else if (request_type == requests::ACTIVE_OFFERS) {
            reply =
                GetSerializer().GetActiveOffers(request[json_field::USER_ID]);
        } else if (request_type == requests::CLOSED_DEALS) {
            reply =
                GetSerializer().GetClosedDeals(request[json_field::USER_ID]);
        } else if (request_type == requests::POST_OFFER) {
            reply = "\"Offer was posted.\"";
            OfferType offer_type =
                request.at(json_field::OFFER_SIDE) == json_field::BUY
                    ? OfferType::BUY
                    : OfferType::SELL;
            GetSerializer().PostOffer(request.at(json_field::USER_ID),
                                      offer_type, request.at(json_field::PRICE),
                                      request.at(json_field::AMOUNT));
        } else if (request_type == requests::QUOTES) {
            reply = GetSerializer().GetQuotes();
        } else if (request_type == requests::CANCEL) {
            reply =
                GetSerializer().CancelOffer(request.at(json_field::USER_ID),
                                            request.at(json_field::OFFER_ID));
        } else {
            reply = "\"ERROR: Unknown request type\"";
        }

        async_write(socket_, buffer(reply, reply.size()),
                    boost::bind(&Session::HandleWrite, this,
                                boost::asio::placeholders::error));
    } else {
        delete this;
    }
}

void Session::HandleWrite(const boost::system::error_code& error) {
    if (!error) {
        socket_.async_read_some(
            buffer(data_, max_length),
            boost::bind(&Session::HandleRead, this, placeholders::error,
                        placeholders::bytes_transferred));
    } else {
        delete this;
    }
}

ip::tcp::socket& Session::GetSocket() { return socket_; }
