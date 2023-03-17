#include "use_cases.h"

namespace app {

ListMapsUseCase::ListMapsUseCase(const Game::Maps& maps) {
    maps_.reserve(maps.size());
    for (const auto& map : maps) {
        maps_.emplace_back(*map.GetId(), map.GetName(), map.GetLootTypes());
    }
}

JoinGameUseCase::JoinGameUseCase(Game& game, Players& players, PlayerTokens& player_tokens, bool is_position_random) 
    : game_(&game)
    , players_(&players)
    , player_tokens_(&player_tokens)
    , is_position_random_(is_position_random)
{}

JoinGameResult JoinGameUseCase::JoinGame (const std::string& map_id, std::string name) {
    if (name.empty()) {
        throw JoinGameError(JoinGameError::InvalidName()); 
    }
    auto id = model::Map::Id(map_id);
    if (!game_->FindMap(id)) {
        throw JoinGameError(JoinGameError::MapNotFound());
    }
    
    auto& session = game_->FindSession(id); 
    auto& dog = session.AddPlayer(name);
    if (is_position_random_) {
        auto rnd_position = GetRandomPosition(map_id);
        dog.SetPosition(rnd_position.x, rnd_position.y);
    } else {
        dog.SetPosition(0, 0);
    }
    auto& player = players_->AddPlayer(session, dog);
    auto token = player_tokens_->AddPlayerToken(player);
    return {token, *player.GetId()};
}

model::DogPosition JoinGameUseCase::GetRandomPosition(const std::string& map_id) {
    auto map = game_->FindMap(model::Map::Id(map_id));
    auto roads = map->GetRoads();
    auto roads_count = roads.horizontal_roads.size() + roads.vertical_roads.size();
    auto n_road = generator_() % roads_count;
    auto pos_x = generator_() % 101;
    auto pos_y = generator_() % 101;
    model::DogPosition position;
    size_t i = 0;
    for (auto& h_roads : roads.horizontal_roads) {
        for (auto& road : h_roads.second) {
            if (i == n_road) {
                position.x = (double(road.GetEnd().x - road.GetStart().x) + 2*constants::ROAD_WIDTH)/100*pos_x + road.GetStart().x - constants::ROAD_WIDTH;
                position.y = double(road.GetEnd().y) - constants::ROAD_WIDTH + 2*constants::ROAD_WIDTH/100*pos_y;
            }
            ++i;
        }
    }

    for (auto& v_road : roads.vertical_roads) {
        for (auto& road : v_road.second) {
            if (i == n_road) {
                position.y = (double(road.GetEnd().y - road.GetStart().y) + 2*constants::ROAD_WIDTH)/100*pos_y + road.GetStart().y - constants::ROAD_WIDTH;
                position.x = double(road.GetEnd().x) - constants::ROAD_WIDTH + 2*constants::ROAD_WIDTH/100*pos_x;
            }
            ++i;
        }
    }
    return position;
}

GameStateUseCase::GameStateUseCase (Game& game, PlayerTokens& player_tokens)
    : game_(&game)
    , player_tokens_(&player_tokens)
{}

GameState GameStateUseCase::GetState(const Token& token) {
    GameState game_state;
    if (auto player = player_tokens_->FindPlayerBy(token)) {
        auto& session = player->GetSession();
        for (const auto& player_dog : session.GetDogs()) {
            PlayerState player_state{
                player_dog.GetId(),
                player_dog.GetPosition(),
                player_dog.GetSpeed(),
                player_dog.GetDirection(),
                player_dog.GetBag()
            };
            game_state.players_states.emplace_back(player_state);
        }
        for (const auto& loot : session.GetLoot()) {
            game_state.loot_states.emplace_back(loot);
        }
    } else {
        throw GameStateError(GameStateError::UnknownToken());
    }
    return game_state;
}

std::string SetPlayerActionUseCase::MovePlayer(const Token &token, std::string move) {
    if (auto player = player_tokens_->FindPlayerBy(token)) {
        auto& dog = player->GetDog();
        auto dog_speed = player->GetSession().GetMap().GetDogSpeed();
        if (move == PlayerActions::MOVE_LEFT) {
            dog.SetSpeed(-dog_speed, 0.0);
            dog.SetDirection(PlayerActions::MOVE_LEFT);
        } else if (move == PlayerActions::MOVE_RIGHT) {
            dog.SetSpeed(dog_speed, 0.0);
            dog.SetDirection(PlayerActions::MOVE_RIGHT);
        } else if (move == PlayerActions::MOVE_UP) {
            dog.SetSpeed(0.0, -dog_speed);
            dog.SetDirection(PlayerActions::MOVE_UP);
        } else if (move == PlayerActions::MOVE_DOWN) {
            dog.SetSpeed(0.0, dog_speed);
            dog.SetDirection(PlayerActions::MOVE_DOWN);
        } else {
            dog.SetSpeed(0.0, 0.0);
        }
    } else {
        throw PlayerActionError(PlayerActionError::UnknownToken());
    }
    return "{}";
}

UpdateGameStateUseCase::UpdateGameStateUseCase(Game& game) 
    : game_(&game) 
    , loot_generator_{TimeInterval(game_->GetLootGenPeriod()), game_->GetLootGenProbability()}                                                    
{}


void UpdateGameStateUseCase::Update(const std::chrono::milliseconds& delta) {
    for (auto& session : game_->GetSessions()) {
        AddLootToSesssion(session, delta);
        UpdateSession(session, delta);
    }
}

MoveDistance UpdateGameStateUseCase::FindMoveDistance(model::Roads& roads, const model::DogPosition& dog_pos, bool is_horizontal) {
    MoveDistance limit;
    int key_x = std::round(dog_pos.x);
    int key_y = std::round(dog_pos.y);

    if (is_horizontal) {
        limit.min = key_x - constants::ROAD_WIDTH;
        limit.max = key_x + constants::ROAD_WIDTH;
        if (roads.horizontal_roads.contains(key_y)) {
            for(auto& road : roads.horizontal_roads[key_y]) {
                double min = road.GetStart().x;
                double max = road.GetEnd().x;
                if (min > max) {
                    std::swap(min, max);
                }
                min -= constants::ROAD_WIDTH;
                max += constants::ROAD_WIDTH;

                if (limit.min > max) {
                    continue;
                } else if (limit.max < min) {
                    break;
                }

                if (limit.min > min) {
                    limit.min = min;
                }

                if (limit.max < max) {
                    limit.max = max;
                }
            }
        }
    } else {
        limit.min = key_y - constants::ROAD_WIDTH;
        limit.max = key_y + constants::ROAD_WIDTH;
        if (roads.vertical_roads.contains(key_x)) {
            for(auto& road : roads.vertical_roads[key_x]) {
                
                double min = road.GetStart().y;
                double max = road.GetEnd().y;
                if (min > max) {
                    std::swap(min, max);
                }
                min -= constants::ROAD_WIDTH;
                max += constants::ROAD_WIDTH;

                if (limit.min > max) {
                    continue;
                } else if (limit.max < min) {
                    break;
                }

                if (limit.min > min) {
                    limit.min = min;
                }

                if (limit.max < max) {
                    limit.max = max;
                }
            }
        }
    }
    return limit;
}

void UpdateGameStateUseCase::AddLootToSesssion(model::GameSession& session, const std::chrono::milliseconds& delta) {
    model::LootState loot_state;
    for (int i = 0; i < loot_generator_.Generate(delta, session.GetLootCount(), session.GetPlayersCount()); ++i) {
        loot_state.type = generator_() % session.GetMap().GetLootTypes().size();
        loot_state.position = GetRandomLootPosition(session);
        session.AddLoot(loot_state);
    }
}

model::LootPosition UpdateGameStateUseCase::GetRandomLootPosition(model::GameSession& session) {
    auto& map = session.GetMap();
    auto roads = map.GetRoads();
    auto roads_count = roads.horizontal_roads.size() + roads.vertical_roads.size();
    auto n_road = generator_() % roads_count;
    auto pos_x = generator_() % 101;
    auto pos_y = generator_() % 101;
    model::LootPosition position;
    size_t i = 0;
    for (auto& h_roads : roads.horizontal_roads) {
        for (auto& road : h_roads.second) {
            if (i == n_road) {
                position.x = (double(road.GetEnd().x - road.GetStart().x) + 2*constants::ROAD_WIDTH)/100*pos_x + road.GetStart().x - constants::ROAD_WIDTH;
                position.y = double(road.GetEnd().y) - constants::ROAD_WIDTH + 2*constants::ROAD_WIDTH/100*pos_y;
            }
            ++i;
        }
    }

    for (auto& v_road : roads.vertical_roads) {
        for (auto& road : v_road.second) {
            if (i == n_road) {
                position.y = (double(road.GetEnd().y - road.GetStart().y) + 2*constants::ROAD_WIDTH)/100*pos_y + road.GetStart().y - constants::ROAD_WIDTH;
                position.x = double(road.GetEnd().x) - constants::ROAD_WIDTH + 2*constants::ROAD_WIDTH/100*pos_x;
            }
            ++i;
        }
    }
    return position;
}

void UpdateGameStateUseCase::UpdateSession(model::GameSession& session, const std::chrono::milliseconds& delta) {
    double dt = double(delta.count()) / 1000;
    auto roads = session.GetMap().GetRoads();
    for (auto& dog : session.GetDogs()) {
        auto dog_pos = dog.GetPosition();
        auto dog_speed = dog.GetSpeed();
        auto direction = dog.GetDirection();
        model::DogPosition new_pos = dog_pos;
        MoveDistance limit;
        if (direction == "L" || direction == "R") {
            limit = FindMoveDistance(roads, dog_pos, true);
            new_pos.x = (dog_pos.x + (dog_speed.vx * dt));
            if (new_pos.x >= limit.max || new_pos.x <= limit.min) {
                if (new_pos.x > limit.max) {
                    new_pos.x = limit.max;
                } else if (new_pos.x < limit.min) {
                    new_pos.x = limit.min;
                }
                dog.SetSpeed(0.0, 0.0);
            }
        } else if (direction == "U" || direction == "D"){
            limit = FindMoveDistance(roads, dog_pos, false);
            new_pos.y = (dog_pos.y + (dog_speed.vy * dt));
            if (new_pos.y >= limit.max || new_pos.y <= limit.min) {
                if (new_pos.y > limit.max) {
                    new_pos.y = limit.max;
                } else if (new_pos.y < limit.min) {
                    new_pos.y = limit.min;
                } 
                dog.SetSpeed(0.0, 0.0);
            }
        }
        dog.SetPosition(new_pos.x, new_pos.y); 
    }
}

} // namespace app