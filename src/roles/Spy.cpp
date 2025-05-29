// Anksilae@gmail.com

#include "Spy.hpp"
#include "Game.hpp"
#include "Exceptions.hpp"
#include <iostream>

namespace coup {

    Spy::Spy(Game& game, const std::string& name)
        : Player(game, name) {}

    std::string Spy::role() const {
        return "Spy";
    }

    

    void Spy::peek_and_disable(Player& target) {
        if (!target.is_active()) {
            throw PlayerAlreadyDeadException(target.get_name());
        }
        if (game->peek_disable==true) {
            throw InvalidActionException("You can only use peek_and_disable once per round.");
        }
        if (target.get_name()==name){
            throw CannotTargetYourselfException("peek_and_disable");
        }
        
      
        
        peeked_coins = target.coins();
        peeked_role = target.role();
        peeked_name = target.get_name();
        std::cout << "[Spy] " << name << " peeked at " << target.get_name() 
                  << "'s coins: " << peeked_coins 
                  << " and role: " << peeked_role << std::endl;
       
            if (game->is_arrest_blocked(target.get_name())) {
                throw InvalidActionException("Arrest is already blocked for this player.");
            }
            game->block_arrest_for(target.get_name());
            std::cout << "[Spy] " << name << " has disabled arrest for " << target.get_name() << std::endl;
            game->perform_action("peek_and_disable", name, target.get_name());
            game->peek_disable=true;
    
    }

} // namespace coup
