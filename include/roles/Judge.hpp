// Anksilae@gmail.com

#pragma once

#include "Player.hpp"

namespace coup {

/**
 * @brief Judge is a role that can undo bribe actions performed by other players.
 */
class Judge : public Player {
public:
    /**
     * @brief Constructs a Judge and registers it in the game.
     * @param game Reference to the game instance.
     * @param name The name of the player.
     */
    Judge(Game& game, const std::string& name);

    /**
     * @brief Returns the role name ("Judge").
     */
    std::string role() const override;

    /**
     * @brief Cancels a bribe action performed by another player and restores its cost.
     * 
     * @param target The player whose bribe should be undone.
     * @throws UndoNotAllowed, InvalidActionException, or CannotTargetYourselfException.
     */
    void undo_bribe(Player& target);
};

} // namespace coup
