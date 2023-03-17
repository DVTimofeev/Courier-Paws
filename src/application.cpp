#include "application.h"

namespace app{
    
Application::Application(const std::filesystem::path& json_path, int tick_delta, bool is_position_random, Strand strand) 
    : game_(json_loader::LoadGame(json_path))
    , strand_(strand) 
    , player_tokens_(PlayerTokens())
    , list_maps_use_case_(game_.GetMaps())
    , join_game_use_case_(game_, players_, player_tokens_, is_position_random)
    , get_map_use_case_(game_)
    , list_players_use_case_(player_tokens_)
    , game_state_use_case_(game_, player_tokens_)
    , set_player_action_use_case_(player_tokens_)
    , update_game_use_case_(game_) 
{
    if (tick_delta) {
        ticker_ = std::make_shared<Ticker>(strand_, std::chrono::milliseconds(tick_delta), [this](std::chrono::milliseconds ms){
            UpdateGame(ms);
        });
        ticker_->Start(); // TODO перенесено в добавление игрока
    }
}

const Map* Application::FindMap(MapId id) {
    return get_map_use_case_.GetMap(id);
}

ListMapsUseCase::Maps Application::ListMaps() {
    return list_maps_use_case_.ListMaps();
}

std::string Application::ListPlayers(const Player& player) {
    return list_players_use_case_.GetPlayersList(player);
}

std::string Application::GetGameState(const Token& token) {
    auto game_state = game_state_use_case_.GetState(token);
    json::object json_body;
    for (const auto& player_state : game_state.players_states) {
        json::array loot_json;
        for (const auto& loot : player_state.bag) {
            loot_json.push_back({{"id", loot.first}, {"type", loot.second}});
        }
        json_body.emplace(
            std::to_string(*player_state.id)
            , json::value{
                {"pos", json::value{player_state.position.x, player_state.position.y}},
                {"speed", json::value{player_state.speed.vx, player_state.speed.vy}},
                {"dir", player_state.direction},
                {"bag", loot_json}
            }
        );
    }
    json::object result;
    result["players"] = json_body;
    json_body.clear();
    for (const auto& loot_state : game_state.loot_states) {
        json_body.emplace(
            std::to_string(loot_state.id)
            , json::value{
                {"type", loot_state.type},
                {"pos", {loot_state.position.x, loot_state.position.y}}
            }
        );
    }
    result["lostObjects"] = json_body;
    return json::serialize(result);
}

Application::Strand& Application::GetApiStrand() {
    return strand_;
}


std::string Application::MovePlayer(const Token& token,const std::string& move) {
    return set_player_action_use_case_.MovePlayer(token, move);
}

void Application::UpdateGame(const std::chrono::milliseconds& delta) {
    update_game_use_case_.Update(delta);
}


std::string Application::GetMapInfo(const Map& map) {
    auto loot_types = GetLootTypes(map.GetLootTypes());
    json::value result({
        {"id", *map.GetId()},
        {"name", map.GetName()},
        // {"roads", ParseRoads(map.GetRoads())},
        {"roads", ParseRoads(map.roads_for_test_)}, // TODO delete after tests;
        {"buildings", ParseBuildings(map.GetBuildings())},
        {"offices", ParseOffices(map.GetOffices())},
        {"lootTypes", loot_types},
    });
    return json::serialize(result);
}

std::string Application::GetMapsSpec() {
    auto maps = ListMaps();
    json::array maps_json;
    for (auto& map : maps){
        maps_json.emplace_back(json::value({
            {"id", map.id},
            {"name", map.name},
            {"lootTypes", GetLootTypes(map.loot_types)}
        }));
    }
    return json::serialize(maps_json);
}

JoinGameResult Application::JoinGame(std::string map_id, std::string name) {
    return join_game_use_case_.JoinGame(map_id, name);
}

Player* Application::FindPlayer(Token token) {
    return player_tokens_.FindPlayerBy(token); //TODO
}

json::array Application::ParseRoads(const model::Roads& roads) const{
    json::array result;
    for (const auto& roads : roads.horizontal_roads) {
        for (const auto& road : roads.second) {
            auto start = road.GetStart();
            auto end = road.GetEnd();
            result.push_back({
                {"x0", start.x},
                {"x1", end.x},
                {"y0", start.y}    
            });
        }
    }

    for (const auto& roads : roads.vertical_roads) {
        for (const auto& road : roads.second) {
            auto start = road.GetStart();
            auto end = road.GetEnd();
            result.push_back({
                {"x0", start.x},
                {"y0", start.y},
                {"y1", end.y}
            });
        }
    }
    return result;
    
}

// TODO delete after all tests passed
json::array Application::ParseRoads(const std::vector<Road>& roads) const{
    json::array result;
    for (auto&& road : roads){
        json::object j;
        auto start = road.GetStart();
        auto end = road.GetEnd();
        j.emplace("x0", start.x);
        if (road.IsHorizontal()) {
            j.emplace("x1", end.x);
        }
        j.emplace("y0", start.y);
        if (road.IsVertical()) {
            j.emplace("y1", end.y);
        }
        result.push_back(j);
    }
    return result;
}

json::array Application::ParseBuildings(const std::vector<Building>& buildings) const{
    json::array result;
    for (auto&& building : buildings){
        json::object j;
        auto bounds = building.GetBounds();
        j.emplace("x", bounds.position.x);
        j.emplace("y", bounds.position.y);
        j.emplace("w", bounds.size.width);
        j.emplace("h", bounds.size.height);
        result.push_back(j);
    }
    return result;
}

json::array Application::ParseOffices(const std::vector<Office>& offices) const{
    json::array result;
    for (auto&& office : offices){
        json::object j;
        auto pos = office.GetPosition();
        auto offset = office.GetOffset();
        j.emplace("id", *office.GetId());
        j.emplace("x", pos.x);
        j.emplace("y", pos.y);
        j.emplace("offsetX", offset.dx);
        j.emplace("offsetY", offset.dy);
        result.push_back(j);
    }
    return result;
}

json::array Application::GetLootTypes(model::Map::LootTypes loot_types) {
    json::array loot_types_json;
    for(auto& loot_type: loot_types) {
        json::object parameters;
        for (const auto& param : loot_type.parameters) {
            if (param.first == "rotation") {
                parameters[param.first] = std::stoi(param.second);
            } else if (param.first == "scale") {
                parameters[param.first] = std::stod(param.second);
            } else {
                parameters[param.first] = param.second;
            }
        }
        loot_types_json.emplace_back(parameters);
    }
    return loot_types_json;
}


} //namespace app
