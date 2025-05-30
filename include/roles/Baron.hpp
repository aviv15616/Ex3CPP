// Anksilae@gmail.com

#pragma once

#include "Player.hpp"

namespace coup {

/**
 * @brief Baron is a special role that can invest coins and gains compensation when sanctioned.
 */
class Baron : public Player {
public:
    /**
     * @brief Constructs a Baron and registers it in the game.
     * @param game Reference to the game instance.
     * @param name The name of the player.
     */
    Baron(Game& game, const std::string& name);

    /**
     * @brief Returns the role name ("Baron").
     */
    std::string role() const override;

    /**
     * @brief Baron's unique action — invest during their turn to gain 3 coins (net profit).
     * 
     * Requires at least 3 coins to activate.
     */
    void invest();

    /**
     * @brief Passive response: When sanctioned, the Baron receives 1 bonus coin as compensation.
     */
    void on_sanction() override;
};

} // namespace coup
