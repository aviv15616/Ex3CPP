// Anksilae@gmail.com

#include "Baron.hpp"
#include "Exceptions.hpp"
#include "Game.hpp"
#include <iostream>

namespace coup {

/**
 * @brief Constructs a Baron role player and registers it to the game.
 * 
 * @param game Reference to the game instance.
 * @param name Name of the player.
 */
Baron::Baron(Game& game, const std::string& name)
    : Player(game, name) {}

/**
 * @brief Returns the role name of this player ("Baron").
 */
std::string Baron::role() const {
    return "Baron";
}

/**
 * @brief Performs the Baron's unique "invest" action.
 * 
 * Adds 3 coins to the player if they have at least 3 coins (net +3 gain).
 * 
 * @throws NotYourTurnException if not the player's turn.
 * @throws MustCoupWith10CoinsException if coup is required.
 * @throws NotEnoughCoinsException if the player has less than 3 coins.
 */
void Baron::invest() {
    if (game->turn() != name) {
        throw NotYourTurnException();
    }
    ensure_coup_required();

    if (coin_count < 3) {
        throw NotEnoughCoinsException(3, coin_count); 
    }

    coin_count += 3;
    game->perform_action("invest", name);
    std::cout << "[Baron] " << name << " invested 3 coins and gained 6. Total: " << coin_count << std::endl;
    game->next_turn();
}

/**
 * @brief Called when the Baron is sanctioned — gains 1 compensation coin.
 */
void Baron::on_sanction() {
    set_coins(coins() + 1);
    under_sanction = true;
    std::cout << "[Baron] " << name << " received 1 coin compensation after sanction. Total: " << coin_count << std::endl;
}

} // namespace coup
