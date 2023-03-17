#include "token.h"

namespace security {

player::Player* PlayerTokens::FindPlayerBy(Token token){
    if (player_by_token_.contains(token)) {
        return &player_by_token_.at(token);
    }
    return nullptr;
}

Token PlayerTokens::AddPlayerToken(player::Player& player){
    uint64_t part1 = generator1_();
    uint64_t part2 = generator2_();
    std::string key;
    std::stringstream ss;
    ss << std::setw(16) << std::setfill('0') << std::hex << part1;
    key += ss.str();
    ss.str("");
    ss << std::setw(16) << std::setfill('0') << std::hex << part2;
    key += ss.str();
    Token token{key};
    player_by_token_.emplace(token, player);
    return token;
    
}

std::optional<Token> TryExtractToken(const StringRequest& request) {
    std::stringstream ss;
    if (request.count(http::field::authorization)) {
        ss << request.at(http::field::authorization);
    }
    std::string auth_type;
    std::string token;
    std::string err;
    ss >> auth_type;
    ss >> token;
    if (auth_type == "Bearer" && token.size() == 32) {
        return Token(token);
    } else {
        return std::nullopt;
    }
}

StringResponse MakeUnauthorizedError() {
    StringResponse response;
    response.result(http::status::unauthorized);
    // response.result(http::status::ok);
    response.set(http::field::cache_control, "no-cache"); 
    response.set(http::field::content_type, ContentType::APP_JSON);
    std::string string_body = R"({"code":"invalidToken", "message":"Authorization header is missing"})"; 
    response.body() = string_body;
    response.content_length(string_body.size());
    return response;
}

} // namespace security