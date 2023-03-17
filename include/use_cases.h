#pragma once

#include <cmath>
#include <chrono>
#include <limits>

// #include <iostream> // TODO delete after tests

#include <boost/json.hpp>

#include "token.h"
#include "constants.h"
#include "loot_generator.h"

namespace app{
using namespace std::literals;
namespace json = boost::json;
using Game = model::Game;
using Player = player::Player;
using Players = player::Players;
using PlayerTokens = security::PlayerTokens;
using Token = security::PlayerTokens::Token;
using Road = model::Road;

struct MapInfo {
    std::string id;
    std::string name;
    model::Map::LootTypes loot_types;
};

class ListMapsUseCase {
public:
    using Maps = std::vector<MapInfo>;

    explicit ListMapsUseCase(const Game::Maps& maps);

    const Maps& ListMaps() {
        return maps_;
    }

private:
    Maps maps_;
};

struct JoinGameResult {
    Token token;
    uint32_t player_id;
};

class JoinGameError : public std::domain_error {
using std::domain_error::domain_error;
public:
    explicit JoinGameError(const std::string& msg) : domain_error(msg) {
    }

    static const char* InvalidName(){
        return R"("{{"code": "invalidArgument"}, {"message": "Invalid name"}}"400)";
    }

    static const char* MapNotFound(){
        return R"("{{"code": "mapNotFound"}, {"message": "Map not found"}}"404)";
    }
};

class JoinGameUseCase {
public:
    explicit JoinGameUseCase(Game& game, Players& players, PlayerTokens& player_tokens, bool is_position_random);

    JoinGameResult JoinGame (const std::string& map_id, std::string name);

private:
    model::DogPosition GetRandomPosition(const std::string& map_id);
    
private:
    Game* game_;
    Players* players_;
    PlayerTokens* player_tokens_;
    bool is_position_random_;

    std::random_device random_device_;
    std::mt19937_64 generator_{[this] {
        std::uniform_int_distribution<uint32_t> dist;
        return dist(random_device_);
    }()};
};

class GetMapUseCase {
public:
    explicit GetMapUseCase(Game& game) : game_(&game){}

    const model::Map* GetMap(model::Map::Id map_id) {
        return game_->FindMap(map_id);
    }

private:
    Game* game_;
};

struct PlayerState {
    player::Dog::Id id;
    model::DogPosition position;
    model::DogSpeed speed;
    std::string direction;
    model::Dog::Bag bag;
};

struct GameState {
    std::vector<PlayerState> players_states;
    std::vector<model::LootState> loot_states;
};

class GameStateError : public std::domain_error {
using std::domain_error::domain_error;
public:
    explicit GameStateError(const std::string& msg) : domain_error(msg) {}

    static const char* UnknownToken(){
        return R"({"code": "unknownToken", "message": "Player token has not been found"}401)";
    }
};

class GameStateUseCase {
public:
    explicit GameStateUseCase (Game& game, PlayerTokens& player_tokens);

    GameState GetState(const Token& token);
private:
    Game* game_;
    PlayerTokens* player_tokens_;
};

class ListPlayersUseCase {
public:
    ListPlayersUseCase(PlayerTokens& player_tokens) : player_tokens_(&player_tokens) {}

    std::string GetPlayersList(const Player& player) {
        json::object players_list;
        for (auto& dog : player.GetSession().GetDogs()) {
            players_list.emplace(std::to_string(*dog.GetId()), json::value{{"name", dog.GetName()}});
        }
        return json::serialize(players_list);
    }
private:
    PlayerTokens* player_tokens_;
};

struct PlayerActions {
    PlayerActions() = delete;
    constexpr static char VALID_MOVES[] = "LRUD";
    constexpr static char MOVE_LEFT[] = "L";
    constexpr static char MOVE_RIGHT[] = "R";
    constexpr static char MOVE_UP[] = "U";
    constexpr static char MOVE_DOWN[] = "D";
    constexpr static char MOVE_STOP[] = "";
};

class PlayerActionError : public std::domain_error {
    using std::domain_error::domain_error;
public:
    explicit PlayerActionError(const std::string& msg) : domain_error(msg) {}

    static const char* UnknownToken(){
        return R"({"code": "unknownToken", "message": "Player token has not been found"}401)";
    }

    static const char* IncorrectMove(){
        return R"({"code": "unknownToken", "message": "Player token has not been found"}401)";
    }
};

class SetPlayerActionUseCase {
public:
    explicit SetPlayerActionUseCase(PlayerTokens& player_tokens) : player_tokens_(&player_tokens) {}

    std::string MovePlayer(const Token &token, std::string move);

private:
    PlayerTokens* player_tokens_;
};

struct MoveDistance
{
    double min;
    double max;
};


class UpdateGameStateUseCase {
public:
    using Position = model::Position;
    using TimeInterval = loot_gen::LootGenerator::TimeInterval;
    explicit UpdateGameStateUseCase(Game& game);

    void Update(const std::chrono::milliseconds& delta);

private:
    const double ROAD_WIDTH = 0.4;
    void UpdateSession(model::GameSession& session, const std::chrono::milliseconds& delta);
    MoveDistance FindMoveDistance(model::Roads& roads, const model::DogPosition& dog_pos, bool is_horizontal);
    void AddLootToSesssion(model::GameSession& session, const std::chrono::milliseconds& delta);
    model::LootPosition GetRandomLootPosition(model::GameSession& session);
private:
    Game* game_;
    loot_gen::LootGenerator loot_generator_;

    std::random_device random_device_;
    std::mt19937_64 generator_{[this] {
        std::uniform_int_distribution<uint32_t> dist;
        return dist(random_device_);
    }()};
};

} // namespace app