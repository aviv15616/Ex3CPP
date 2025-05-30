// Anksilae@gmail.com

#pragma once

#include "Player.hpp"

namespace coup {

/**
 * @brief Merchant is a role that gains a bonus coin at the start of their turn if they have 3 or more coins.
 */
class Merchant : public Player {
public:
    /**
     * @brief Constructs a Merchant and registers it in the game.
     * @param game Reference to the game instance.
     * @param name The name of the player.
     */
    Merchant(Game& game, const std::string& name);

    /**
     * @brief Returns the role name ("Merchant").
     */
    std::string role() const override;

    /**
     * @brief Grants 1 bonus coin at the start of the turn if the Merchant has at least 3 coins.
     */
    void on_turn_start() override;
};

} // namespace coup
