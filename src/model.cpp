#include "model.h"

#include <stdexcept>

namespace model {
using namespace std::literals;

void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

void Map::AddRoad(const Road& road) {
    roads_for_test_.push_back(road); // TODO to delete
    if (road.IsHorizontal()) {
        roads_.horizontal_roads[road.GetStart().y].emplace(road);
    } else {
        roads_.vertical_roads[road.GetStart().x].emplace(road);
    }
}

const Map* Game::FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }


void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

Dog& GameSession::AddPlayer(std::string name) {
    dogs_.emplace_back(Dog(name, players_counter_++));
    return dogs_.back();
}

void GameSession::AddLoot(LootState loot) {
    loot.id = loot_counter_++;
    loot_.emplace_back(loot);
}

}  // namespace model
