#pragma once

#include <optional>
#include <string>
#include <sstream>
#include <iomanip>

#include "model.h"

namespace player {

using GameSession = model::GameSession;
using Dog = model::Dog;


class Player{
public:
    using Id = util::Tagged<uint32_t, Player>;
    Player(Id id, GameSession& session, Dog& dog);

    Id GetId() { return id_;}

    Dog::Id GetDogId() const {
        return dog_.GetId();
    }

    Dog& GetDog() const {
        return dog_;
    }

    GameSession& GetSession() const {
        return session_;
    }
    
private:
    Id id_;
    Dog& dog_; 
    GameSession& session_;

};

class Players{
public:
    using PlayerById = std::unordered_map<Player::Id, Player, util::TaggedHasher<Player::Id>>;

    Player& AddPlayer(GameSession& session, Dog& dog);
private:
    PlayerById player_by_id_;
    uint32_t player_counter_ = 0;
};



} // namespace player