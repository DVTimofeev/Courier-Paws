#pragma once
// #include <optional>

#include "response.h"
#include "http_server.h"
#include "application.h"
#include "uri_api.h"

namespace api_handler{
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
using StringRequest = http::request<http::string_body>;
using StringResponse = http::response<http::string_body>;
using GameSession = model::GameSession;
using Game = model::Game;
using Players = player::Players;
using PlayerTokens = security::PlayerTokens;
using Token = PlayerTokens::Token;
using Road = model::Road;
using Building = model::Building;
using Office = model::Office;
using Dog = model::Dog;
using Map = model::Map;
using Application = app::Application;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view APP_JSON = "application/json"sv;
};

struct ErrorStatus {
    ErrorStatus() = delete;
    constexpr static std::string_view BAD_REQUEST = "400"sv;
    constexpr static std::string_view UNAUTHORIZED = "401"sv;
    constexpr static std::string_view FORBIDDEN = "403"sv;
    constexpr static std::string_view NOT_FOUND = "404"sv;
    constexpr static std::string_view METHOD_NOT_ALLOWED = "405"sv;
};

class ApiHandler{
public:
    ApiHandler(Application& app);
    bool IsApiRequest(const StringRequest& req) const;
    StringResponse HandleRequest(const StringRequest& req);

private:
    void LinkJoinWithoutAuthorize();
    void LinkPlayerList();
    void LinkGameState();
    void LinkGameTick();
    void LinkPlayerAction();

    StringResponse ProcessPostEndpointWithoutAuthorization(std::string_view body){
        return StringResponse();
    }
    StringResponse AddPlayer(std::string_view body);
    StringResponse GetPlayers(const Token& token, std::string_view body);
    StringResponse GetGameState(const Token& token, std::string_view body);
    StringResponse UpdateGameState(std::string_view body);
    StringResponse SetPlayerAction(const Token& token, std::string_view body);
    StringResponse HandleMapsRequest(const StringRequest& request);
    bool ValidatePlayerMove(const std::string_view& move);

    std::vector<std::string> GetParsedTarget(std::string_view target) const;

    json::array parse_roads(const std::vector<Road>& roads) const;

    json::array parse_buildings(const std::vector<Building>& buildings) const;

    json::array parse_offices(const std::vector<Office>& offices) const;

public: 
    Application& app_;
    uri_api::UriData uri_handler_;
};

} // namespase api_handler