// Anksilae@gmail.com

#pragma once

#include "Player.hpp"

namespace coup {

/**
 * @brief Governor is a role that performs an enhanced tax action and can undo tax actions of others.
 */
class Governor : public Player {
public:
    /**
     * @brief Constructs a Governor and registers it in the game.
     * @param game Reference to the game instance.
     * @param name The name of the player.
     */
    Governor(Game& game, const std::string& name);

    /**
     * @brief Returns the role name ("Governor").
     */
    std::string role() const override;

    /**
     * @brief Special tax action — takes 3 coins instead of 2.
     * @throws NotYourTurnException or InvalidActionException if invalid.
     */
    void tax() override;

    /**
     * @brief Allows the Governor to undo a tax action performed by another player.
     * 
     * @param target The player whose tax should be undone.
     * @throws UndoNotAllowed, InvalidActionException, or CannotTargetYourselfException.
     */
    void undo_tax(Player& target);
};

} // namespace coup
