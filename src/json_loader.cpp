#include "json_loader.h"

namespace {

model::LootType ConvertJsonToLootType(boost::json::value loot_json) {
    model::LootType loot;
    for (const auto& param : loot_json.as_object()) {
        std::string value;
        if (param.value().is_int64()) {
            value = std::to_string(param.value().as_int64());
        } else if (param.value().is_double()) {
            value = std::to_string(param.value().as_double());
        } else {
            value = param.value().as_string();
        }
        loot.parameters.emplace(param.key(), value);     
    }
    return loot;
}

} // namespace {}

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path) {
    std::ifstream jsonFile(json_path);
    std::string input(std::istreambuf_iterator<char>(jsonFile), {});
    auto configJSON = boost::json::parse(input);
    jsonFile.close();

    model::Game game;
    auto default_dog_speed = GetDefaultDogSpeed(configJSON);
    auto default_bag_capacity = GetDefaultBagCapacity(configJSON);
    auto loot_gen_properties = GetLootGenProp(configJSON);
    game.SetLootGenPeriod(loot_gen_properties.perion);
    game.SetLootGenProbability(loot_gen_properties.probability);
    for (auto&& json_map : configJSON.at("maps").as_array()) {
        game.AddMap(LoadMap(json_map, default_dog_speed, default_bag_capacity)); 
    }
    return game;
}

LootGeneratorProperties GetLootGenProp(const json::value& config) {
    int loot_generating_period = constants::DEFAULT_LOOT_GENERATE_PERIOD;
    double loot_generating_probability = constants::DEFAULT_LOOT_GENERATE_PROBABILITY;
    if (config.as_object().contains("lootGeneratorConfig")) {
        if (config.at("lootGeneratorConfig").as_object().contains("period")) {
            loot_generating_period = config.at("lootGeneratorConfig").at("period").as_double() * 1000; // s -> ms
        }

        if (config.at("lootGeneratorConfig").as_object().contains("probability")) {
            loot_generating_probability = config.at("lootGeneratorConfig").at("probability").as_double();
        }
    } 

    return {loot_generating_period, loot_generating_probability};
}

model::Speed GetDefaultDogSpeed(const json::value& config) {
    model::Speed default_dog_speed = constants::DEFAULT_DOG_SPEED;
    if (config.as_object().contains("defaultDogSpeed")) {
        default_dog_speed = config.at("defaultDogSpeed").as_double();
    }
    return default_dog_speed;
}

int GetDefaultBagCapacity(const json::value& config) {
    int default_bag_capacity = constants::DEFAULT_BAG_CAPACITY;
    if (config.as_object().contains("defaultBagCapacity")) {
        default_bag_capacity = int(config.at("defaultBagCapacity").as_int64());
    }
    return default_bag_capacity;
}

model::Map LoadMap(const json::value& json_map, model::Speed default_dog_speed, int default_bag_capacity) {
    util::Tagged<std::string, model::Map> id{json_map.at("id").as_string().c_str()};
    model::Map map(id, json_map.at("name").as_string().c_str());
    AddRoadToMap(map, json_map);
    AddBuildingsToMap(map, json_map);
    AddOfficesToMap(map, json_map);
    AddDogSpeedToMap(map, json_map, default_dog_speed);
    AddBagCapacityToMap(map, json_map, default_bag_capacity);
    AddLootTypeToMap(map, json_map);
    return map;
}

void AddRoadToMap(model::Map& map, const json::value& json_map) {
    for (auto&& road : json_map.at("roads").as_array()){
        model::Point start{road.at("x0").as_int64(), road.at("y0").as_int64()};
        if (road.as_object().if_contains("x1")){
            map.AddRoad(model::Road(model::Road::HORIZONTAL, start, road.at("x1").as_int64()));
        } else {
            map.AddRoad(model::Road(model::Road::VERTICAL, start, road.at("y1").as_int64()));
        }
    }
    // TODO код ниже для идентифткации сколько дорог зарегружено, была проблема что сет 2 дороги не загружал. 
    // мультисет справляется но пусть код пока полежит
    int count = 0;
    for (auto road_set : map.GetRoads().horizontal_roads) {
        count += road_set.second.size();
    }
    for (auto road_set : map.GetRoads().vertical_roads) {
        count += road_set.second.size();
    }

    std::cout << count << std::endl;
    std::cout << map.roads_for_test_.size() << std::endl;
}
void AddBuildingsToMap(model::Map& map, const json::value& json_map) {
    for (auto&& building : json_map.at("buildings").as_array()){
        model::Point position{building.at("x").as_int64(), building.at("y").as_int64()};
        model::Size size{building.at("w").as_int64(), building.at("h").as_int64()};
        model::Rectangle rectangle{position, size};
        map.AddBuilding(model::Building(rectangle));
    }
}
void AddOfficesToMap(model::Map& map, const json::value& json_map) {
    for (auto&& office : json_map.at("offices").as_array()){
        util::Tagged<std::string, model::Office> id{office.at("id").as_string().c_str()};
        model::Point position{office.at("x").as_int64(), office.at("y").as_int64()};
        model::Offset offset{office.at("offsetX").as_int64(), office.at("offsetY").as_int64()};
        map.AddOffice(model::Office(id, position, offset));
    }
}
void AddDogSpeedToMap(model::Map& map, const json::value& json_map, model::Speed default_dog_speed) {
    if (json_map.as_object().contains("dogSpeed")) {
        map.SetDogSpeed(json_map.at("dogSpeed").as_double());
    } else {
        map.SetDogSpeed(default_dog_speed);
    }
}
void AddBagCapacityToMap(model::Map& map, const json::value& json_map, int default_bag_capacity) {
    if (json_map.as_object().contains("bagCapacity")) {
        map.SetBagCapacity(int(json_map.at("bagCapacity").as_int64()));
    } else {
        map.SetBagCapacity(default_bag_capacity);
    }
}

void AddLootTypeToMap(model::Map& map, const json::value& json_map) {
    for (auto&& loot_type : json_map.at("lootTypes").as_array()){
        map.AddLootType(ConvertJsonToLootType(loot_type));
    }
}

}  // namespace json_loader
