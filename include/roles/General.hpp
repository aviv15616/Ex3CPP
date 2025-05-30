// Anksilae@gmail.com

#pragma once

#include "Player.hpp"

namespace coup {

/**
 * @brief General is a role that can undo a coup and receives compensation upon arrest.
 */
class General : public Player {
public:
    /**
     * @brief Constructs a General and registers it in the game.
     * @param game Reference to the game instance.
     * @param name The name of the player.
     */
    General(Game& game, const std::string& name);

    /**
     * @brief Returns the role name ("General").
     */
    std::string role() const override;

    /**
     * @brief Cancels a coup performed against a player (including self) at a cost of 5 coins.
     * 
     * Can only be used once per round.
     * @param target The player whose coup should be undone.
     */
    void undo_coup(Player& target);

    /**
     * @brief Passive effect: when the General is arrested, they get back the coin taken.
     */
    void on_arrest() override;
};

} // namespace coup
