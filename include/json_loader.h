#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>

#include <boost/json.hpp>

#include "model.h"
#include "constants.h"

namespace json_loader {

namespace json = boost::json;

struct LootGeneratorProperties {
    int perion;
    double probability;
};

model::Game LoadGame(const std::filesystem::path& json_path);

LootGeneratorProperties GetLootGenProp(const json::value& config);
model::Speed GetDefaultDogSpeed(const json::value& config);
int GetDefaultBagCapacity(const json::value& config);
model::Map LoadMap(const json::value& json_map, model::Speed default_dog_speed, int default_bag_capacity);
void AddRoadToMap(model::Map& map, const json::value& json_map);
void AddBuildingsToMap(model::Map& map, const json::value& json_map);
void AddOfficesToMap(model::Map& map, const json::value& json_map);
void AddDogSpeedToMap(model::Map& map, const json::value& json_map, model::Speed default_dog_speed);
void AddBagCapacityToMap(model::Map& map, const json::value& json_map, int default_bag_capacity);
void AddLootTypeToMap(model::Map& map, const json::value& json_map);

}  // namespace json_loader
