#pragma once

#include <boost/asio.hpp>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include "json.h"

enum RequestType {
    POST_OFFER = 1,
    CANCEL_OFFER = 2,
    GET_QUOTES = 3,
    GET_BALANCE = 4,
    GET_ACTIVE = 5,
    GET_CLOSED = 6,

    FIRST = POST_OFFER,
    LAST = GET_CLOSED,
};

class RequestHandler {
   public:
    RequestHandler(boost::asio::ip::tcp::socket& socket,
                   const std::string& request_type, uint64_t user_id);

    void Handle();

    virtual ~RequestHandler() = default;

   protected:
    virtual nlohmann::json SendRequest() = 0;

    virtual void PrintResult(const nlohmann::json& response) = 0;

    nlohmann::json ReadResponse();

   protected:
    boost::asio::ip::tcp::socket& socket_;
    std::string request_type_;
    uint64_t user_id_;
};

class IdOnlyRequestHandler : public RequestHandler {
   public:
    using RequestHandler::RequestHandler;

    virtual ~IdOnlyRequestHandler() = default;

   private:
    nlohmann::json SendRequest() override;
};

class WithPrerequisitesRequestHandler : public RequestHandler {
   public:
    using RequestHandler::RequestHandler;

    virtual ~WithPrerequisitesRequestHandler() = default;

   protected:
    virtual void GatherPrerequisites() = 0;
};

class GetQuotesHandler final : public IdOnlyRequestHandler {
   public:
    using IdOnlyRequestHandler::IdOnlyRequestHandler;

   private:
    void PrintResult(const nlohmann::json& response) override;

    std::string NullableIntToString(const nlohmann::json& nullable_int);
};

class GetBalanceRequest final : public IdOnlyRequestHandler {
   public:
    using IdOnlyRequestHandler::IdOnlyRequestHandler;

   private:
    void PrintResult(const nlohmann::json& response) override;
};

class GetActiveOffersRequest final : public IdOnlyRequestHandler {
   public:
    using IdOnlyRequestHandler::IdOnlyRequestHandler;

   private:
    void PrintResult(const nlohmann::json& response) override;
};

class GetClosedDealsRequest final : public IdOnlyRequestHandler {
   public:
    using IdOnlyRequestHandler::IdOnlyRequestHandler;

   private:
    void PrintResult(const nlohmann::json& response) override;
};

class PostOfferRequest final : public WithPrerequisitesRequestHandler {
   public:
    using WithPrerequisitesRequestHandler::WithPrerequisitesRequestHandler;

   private:
    nlohmann::json SendRequest() override;

    void GatherPrerequisites() override;

    void PrintResult(const nlohmann::json& response) override;

   private:
    int offer_side_;
    int amount_;
    int price_;
};

class CancleOfferRequest final : public WithPrerequisitesRequestHandler {
   public:
    using WithPrerequisitesRequestHandler::WithPrerequisitesRequestHandler;

   private:
    nlohmann::json SendRequest() override;

    void GatherPrerequisites() override;

    void PrintResult(const nlohmann::json& response) override;

   private:
    int offer_id_;
};

std::unique_ptr<RequestHandler> MakeRequest(
    RequestType type, boost::asio::ip::tcp::socket& socket, uint64_t user_id);
