#include "player.h"

namespace player{

Player::Player(Id id, GameSession& session, Dog& dog) 
        : id_(std::move(id))
        , session_(session)
        , dog_(dog) 
    {}

Player& Players::AddPlayer(GameSession& session, Dog& dog){
        Player::Id id{player_counter_++};
        player_by_id_.emplace(id, Player(id, session, dog));
        return player_by_id_.at(id);
    }

} // namespace player