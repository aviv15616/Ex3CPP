// Anksilae@gmail.com

#pragma once

#include "Player.hpp"

namespace coup {

/**
 * @brief Spy is a role that can peek at another player's role and coin count, and block their ability to arrest.
 */
class Spy : public Player {
private:
    int peeked_coins = 0;       ///< Number of coins seen during peek (not used externally)
    std::string peeked_role;    ///< Role seen during peek (not used externally)
    std::string peeked_name;    ///< Player name seen during peek (not used externally)

public:
    /**
     * @brief Constructs a Spy and registers it in the game.
     * @param game Reference to the game instance.
     * @param name The name of the player.
     */
    Spy(Game& game, const std::string& name);

    /**
     * @brief Returns the role name ("Spy").
     */
    std::string role() const override;

    /**
     * @brief Peeks at the target player's role and coins, and disables their ability to arrest on the next turn.
     * 
     * Can only be used once per round. Cannot target self.
     * 
     * @param target The player to peek at and disable.
     * @throws PlayerAlreadyDeadException, CannotTargetYourselfException, or InvalidActionException.
     */
    void peek_and_disable(Player& target);
};

} // namespace coup
