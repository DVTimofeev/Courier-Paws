#pragma once

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <random>
#include <unordered_map>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/format.hpp>

#include "tagged.h"
#include "player.h"
#include "constants.h"


namespace security {

namespace beast = boost::beast;
namespace http = beast::http;
using StringRequest = http::request<http::string_body>;
using StringResponse = http::response<http::string_body>;

namespace detail {
struct TokenTag {
public:
    explicit TokenTag(std::string hex_token) noexcept
        : hex_token_{hex_token} {
    }

    const std::string& GetHexToken() const noexcept {
        return hex_token_;
    }

private:
    std::string hex_token_;
};

}  // namespace detail

class PlayerTokens{
public:
    using Token = util::Tagged<std::string, detail::TokenTag>;
    using PlayerByToken = std::unordered_map<Token, player::Player&, util::TaggedHasher<Token>>;

    player::Player* FindPlayerBy(Token token);

    Token AddPlayerToken(player::Player& player);

private:
// для токена
    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};

    PlayerByToken player_by_token_;
};


using Token = PlayerTokens::Token;

std::optional<Token> TryExtractToken(const StringRequest& request);

StringResponse MakeUnauthorizedError();

template <typename Fn>
StringResponse ExecuteAuthorized(const StringRequest& request, Fn&& action) {
    if (auto token = TryExtractToken(request)) {
        return action(*token, request.body());
    } else {
        return MakeUnauthorizedError();
    }
}

} // namespace security