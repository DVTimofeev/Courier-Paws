#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include "json_loader.h"
#include "use_cases.h"
#include "ticker.h"


namespace app{
namespace json = boost::json;

using Game = model::Game;
using Road = model::Road;
using Building = model::Building;
using Office = model::Office;
using Player = player::Player;
using Players = player::Players;
using PlayerTokens = security::PlayerTokens;
using Token = PlayerTokens::Token;
using Map = model::Map;
using Maps = Game::Maps;
using MapId = Map::Id;

class Application{
public:
    using Strand = boost::asio::strand<boost::asio::io_context::executor_type>;
    
    Application(const std::filesystem::path& json_path, int tick_delta, bool is_position_random, Strand strand);
    Strand& GetApiStrand();
    ListMapsUseCase::Maps ListMaps();
    const Map* FindMap(MapId id);
    JoinGameResult JoinGame(std::string map_id, std::string name);
    Player* FindPlayer(Token token);
    std::string GetMapInfo(const Map& map);
    std::string GetMapsSpec();
    std::string ListPlayers(const Player& player);
    std::string GetGameState(const Token& token);
    std::string MovePlayer(const Token& token,const std::string& move);
    void UpdateGame(const std::chrono::milliseconds& delta);
    bool IsTickRequestAllowed() {
        return is_tick_request_allowed_;
    }



private:
    json::array ParseRoads(const model::Roads& roads) const;
    json::array ParseRoads(const std::vector<Road>& roads) const; // TODO delete after tests
    json::array ParseBuildings(const std::vector<Building>& buildings) const;
    json::array ParseOffices(const std::vector<Office>& offices) const;
    json::array GetLootTypes(model::Map::LootTypes loot_types);

    Strand strand_;

    Game game_;
    Players players_;
    PlayerTokens player_tokens_;

    std::shared_ptr<Ticker> ticker_;
    bool is_tick_request_allowed_ = true;
    bool is_game_started_ = false;

    // use_cases
    ListMapsUseCase list_maps_use_case_;
    ListPlayersUseCase list_players_use_case_;
    JoinGameUseCase join_game_use_case_;
    GetMapUseCase get_map_use_case_;
    GameStateUseCase game_state_use_case_;
    SetPlayerActionUseCase set_player_action_use_case_;
    UpdateGameStateUseCase update_game_use_case_;
};

} // namespace app