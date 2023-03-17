#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <set>

#include "tagged.h"

namespace model {

using Dimension = int64_t;
using Coord = Dimension;
using Position = double;
using Speed = double;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

struct DogPosition {
    Position x, y;
};

struct LootPosition {
    Position x, y;
};

struct DogSpeed {
    Speed vx, vy;
};

class Road {
    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };

    struct VerticalTag {
        explicit VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    // для использования в сете и автоматической сортировки
    friend bool operator<(const Road& l, const Road& r) {
        if (l.IsHorizontal() && r.IsHorizontal()) {
            return l.GetEnd().x <= r.GetStart().x;
        }

        if (l.IsVertical() && r.IsVertical()) {
            return l.GetEnd().y <= r.GetStart().y;
        }
        return false;
    }

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y} {
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y} {
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }

private:
    Point start_;
    Point end_;
};

struct Roads {
    using HorizontalByY = std::unordered_map<int, std::set<Road>>;
    using VerticalbyX = std::unordered_map<int, std::set<Road>>;
    
    HorizontalByY horizontal_roads;
    VerticalbyX vertical_roads;
};

class Building {
public:
    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }

private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}
        , position_{position}
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

struct LootType {
    std::unordered_map<std::string, std::string> parameters;
};

struct LootState
{
    int id;
    int type;
    model::LootPosition position;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;
    using LootTypes = std::vector<LootType>;

    Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    const LootTypes& GetLootTypes() const noexcept {
        return loot_types_;
    }

    void AddRoad(const Road& road);

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }
    
    void SetDogSpeed(Speed speed) {
        dog_speed_ = speed;
    }

    Speed GetDogSpeed() const {
        return dog_speed_;
    }

    void SetBagCapacity(int bag_capacity) {
        bag_capacity_ = bag_capacity;
    }

    int GetBagCapacity() const {
        return bag_capacity_;
    }

    void AddOffice(Office office);
    
    void AddLootType(const LootType& loot_type) {
        loot_types_.emplace_back(loot_type);
    }
    
    std::vector<Road> roads_for_test_; // TODO DELETE this after test is over
private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;
    Speed dog_speed_;
    int bag_capacity_;
    LootTypes loot_types_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

class Dog {
public:
    using Id = util::Tagged<uint32_t, Dog>;
    using Bag = std::unordered_map<int,int>;
    Dog(std::string name, uint32_t id) 
        : name_(name)
        , id_(id)
        , speed_{0.0, 0.0} 
        {}

    std::string GetName() {
        return name_;
    }

    Id GetId() const {
        return id_;
    }

    std::string GetDirection() const {
        return dir_;
    }

    const DogPosition GetPosition() const {
        return pos_;
    }

    const DogSpeed GetSpeed() const {
        return speed_;
    }

    void SetDirection(std::string dir) {
        dir_ = dir;
    }

    void SetPosition(double x, double y) {
        pos_.x = x;
        pos_.y = y;
    }

    void SetSpeed(double vx, double vy) {
        speed_.vx = vx;
        speed_.vy = vy;
    }

    void AddLoot(const LootState& loot) {
        bag_[loot.id] = loot.type;
    }

    Bag GetBag() const {
        return bag_;
    }

    void BagClear() {
        bag_.clear();
    }

private:
    std::string name_;
    std::string dir_ = "U";
    DogPosition pos_;
    DogSpeed speed_;
    Bag bag_;
    Id id_;
};

class GameSession {
public:
    GameSession(Map& map) : map_(map){}
    Dog& AddPlayer(std::string name);

    void AddLoot(LootState loot);

    const Map::Id& GetMapId() {
        return map_.GetId();
    }

    Map& GetMap() const noexcept {
        return map_;
    }

    std::deque<Dog>& GetDogs() {
        return dogs_;
    }

    std::deque<LootState>& GetLoot() {
        return loot_;
    }

    uint32_t GetPlayersCount() const noexcept {
        return players_counter_;
    }

    uint32_t GetLootCount() const noexcept {
        return loot_counter_;
    }
private:
    std::deque<Dog> dogs_;
    std::deque<LootState> loot_;
    Map& map_;
    uint32_t players_counter_ = 0;
    uint32_t loot_counter_ = 0;
};

class Game {
public:
    using Maps = std::vector<Map>;
    using Sessions = std::vector<GameSession>;

    void AddMap(Map map);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }
    const Map* FindMap(const Map::Id& id) const noexcept;

    // был модификатор конст, но я его убрал т.к. сессия теперь должна изменяться со временем (статус игроков и координаты)
    Sessions& GetSessions() noexcept {
        return sessions_;
    }

    GameSession& FindSession(const Map::Id& id) {
        // поиск сессии
        for (auto&& session : sessions_){
            if(id == session.GetMapId()) {
                return session;
            }
        }
        // Если открытой сессии не нашлось, то нужно создать новую и отдать.
        sessions_.emplace_back(GameSession(maps_[map_id_to_index_.at(id)]));
        return sessions_.back();
    }

    void SetLootGenPeriod(int milliseconds) {
        loot_generating_period_ = milliseconds;
    }

    void SetLootGenProbability(double probability) {
        loot_generating_probability_ = probability;
    }

    int GetLootGenPeriod() const noexcept {
        return loot_generating_period_;
    }

    double GetLootGenProbability() const noexcept {
        return loot_generating_probability_;
    }
    
private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    Maps maps_;
    MapIdToIndex map_id_to_index_;
    Sessions sessions_;

    int loot_generating_period_;
    double loot_generating_probability_;
};
}  // namespace model
