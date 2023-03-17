#pragma once

#include <string>
#include <unordered_set>

using namespace std::literals;


namespace constants {

const double ROAD_WIDTH = 0.4;
const double DEFAULT_DOG_SPEED = 1.0;
const int DEFAULT_BAG_CAPACITY = 3;
const int DEFAULT_LOOT_GENERATE_PERIOD = 5000;
const double DEFAULT_LOOT_GENERATE_PROBABILITY = 0.5;


struct LootType
{
    // allowed parameters of LootTypes
    const std::unordered_set<std::string> ALLOWED_PARAMETERS = {
        "name",
        "file",
        "type",
        "rotation",
        "color",
        "scale"
    };
};

} // namespace constants

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view TEXT_CSS = "text/css"sv;
    constexpr static std::string_view TEXT_PLAIN = "text/plain"sv;
    constexpr static std::string_view TEXT_JS = "text/javascript"sv;
    constexpr static std::string_view APP_JSON = "application/json"sv;
    constexpr static std::string_view APP_XML = "application/xml"sv;
    constexpr static std::string_view APP_OCTET_STREAM = "application/octet-stream"sv;
    constexpr static std::string_view IMAGE_PNG = "image/png"sv;
    constexpr static std::string_view IMAGE_JPEG = "image/jpeg"sv;
    constexpr static std::string_view IMAGE_GIF = "image/gif"sv;
    constexpr static std::string_view IMAGE_BMP = "image/bmp"sv;
    constexpr static std::string_view IMAGE_ICO = "image/vnd.microsoft.icon"sv;
    constexpr static std::string_view IMAGE_TIFF = "image/tiff"sv;
    constexpr static std::string_view IMAGE_SVG_XML = "image/svg+xml"sv;
    constexpr static std::string_view AUDIO_MPEG = "audio/mpeg"sv;
};

struct ErrorCode {
    static inline constexpr std::string_view BAD_REQUEST = "badRequest"sv;
    static inline constexpr std::string_view INVALID_METHOD = "invalidMethod"sv;
    static inline constexpr std::string_view INVALID_ARGUMENT = "invalidArgument"sv;
    static inline constexpr std::string_view INVALID_TOKEN = "invalidToken"sv;
    static inline constexpr std::string_view UNKNOWN_TOKEN = "unknownToken"sv;
};

struct ErrorMessage {
    static inline constexpr std::string_view BAD_REQUEST = "Bad request"sv;
    static inline constexpr std::string_view INVALID_ENDPOINT = "Invalid endpoint"sv;
    static inline constexpr std::string_view POST_IS_EXPECTED = "Only POST method is expected"sv;
    static inline constexpr std::string_view GET_IS_EXPECTED = "Only GET method is expected"sv;
    static inline constexpr std::string_view INVALID_TOKEN = "Authorization header is missing"sv;
    static inline constexpr std::string_view UNKNOWN_TOKEN = "Player token has not been found"sv;
};

struct MiscMessage {
    static inline constexpr std::string_view ALLOWED_POST_METHOD = "POST"sv;
    static inline constexpr std::string_view ALLOWED_GET_HEAD_METHOD = "GET, HEAD"sv;
};


struct Endpoint {
    static inline constexpr std::string_view API = "/api/"sv;
    static inline constexpr std::string_view GAME = "/api/v1/game/"sv;
    static inline constexpr std::string_view MAPS = "/api/v1/maps/"sv;
    static inline constexpr std::string_view JOIN_GAME = "/api/v1/game/join"sv;
    static inline constexpr std::string_view PLAYER_LIST = "/api/v1/game/players"sv;
    static inline constexpr std::string_view GAME_STATE = "/api/v1/game/state"sv;
    static inline constexpr std::string_view GAME_TICK = "/api/v1/game/tick"sv;
    static inline constexpr std::string_view PLAYER_ACTION = "/api/v1/game/player/action"sv;
};


