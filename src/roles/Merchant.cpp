// Anksilae@gmail.com

#include "Merchant.hpp"
#include "Exceptions.hpp"
#include "Game.hpp"
#include <iostream>

namespace coup {

/**
 * @brief Constructs a Merchant role player and registers it to the game.
 * 
 * @param game Reference to the game instance.
 * @param name Name of the player.
 */
Merchant::Merchant(Game& game, const std::string& name)
    : Player(game, name) {}

/**
 * @brief Returns the role name of this player ("Merchant").
 */
std::string Merchant::role() const {
    return "Merchant";
}

/**
 * @brief At the start of the Merchant's turn, gains 1 bonus coin if they have at least 3 coins.
 * 
 * This is a passive ability specific to the Merchant role.
 */
void Merchant::on_turn_start() {
    if (coins() >= 3) {
        set_coins(coins() + 1);
        std::cout << "[Merchant] " << name << " gained 1 bonus coin at start of turn. Total: " << coins() << std::endl;
    }
}

} // namespace coup
