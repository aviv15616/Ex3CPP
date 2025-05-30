// Anksilae@gmail.com

#include "Spy.hpp"
#include "Game.hpp"
#include "Exceptions.hpp"
#include <iostream>

namespace coup {

/**
 * @brief Constructs a Spy role player and registers it to the game.
 * 
 * @param game Reference to the game instance.
 * @param name Name of the player.
 */
Spy::Spy(Game& game, const std::string& name)
    : Player(game, name) {}

/**
 * @brief Returns the role name of this player ("Spy").
 */
std::string Spy::role() const {
    return "Spy";
}

/**
 * @brief Allows the Spy to peek at a target’s coins and role, and block their ability to arrest.
 * 
 * Can be used only once per round. Cannot be used on self or on dead players.
 * Also ensures arrest is not already blocked.
 * 
 * @param target The player to peek at and disable arrest for.
 * @throws PlayerAlreadyDeadException if the target is not active.
 * @throws InvalidActionException if the ability was already used this round or arrest is already blocked.
 * @throws CannotTargetYourselfException if targeting self.
 */
void Spy::peek_and_disable(Player& target) {
    if (!target.is_active()) {
        throw PlayerAlreadyDeadException(target.get_name());
    }
    if (game->peek_disable == true) {
        throw InvalidActionException("You can only use peek_and_disable once per round.");
    }
    if (target.get_name() == name) {
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
    game->peek_disable = true;
}

} // namespace coup
