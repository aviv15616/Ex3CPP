// Anksilae@gmail.com

#include "Judge.hpp"
#include "Exceptions.hpp"
#include "Game.hpp"
#include "Player.hpp"
#include <iostream>

namespace coup {

/**
 * @brief Constructs a Judge role player and registers it to the game.
 * 
 * @param game Reference to the game instance.
 * @param name Name of the player.
 */
Judge::Judge(Game& game, const std::string& name)
    : Player(game, name) {}

/**
 * @brief Returns the role name of this player ("Judge").
 */
std::string Judge::role() const {
    return "Judge";
}

/**
 * @brief Undoes a bribe action performed by another player.
 * 
 * Can only be used once per round. Cannot be used on self.
 * 
 * @param target The player whose bribe action will be undone.
 * @throws UndoNotAllowed if the target's last action wasn't bribe.
 * @throws InvalidActionException if already undone this round.
 * @throws CannotTargetYourselfException if trying to undo own bribe.
 */
void Judge::undo_bribe(Player& target) {
    if (!game->can_undo_action(target.get_name(), "bribe")) {
        throw UndoNotAllowed(role(), "undo_bribe");
    }
    if (game->undo_bribe) {
        throw InvalidActionException("Bribe already undone this round.");
    }
    if (target.get_name() == name) {
        throw CannotTargetYourselfException("undo bribe");        
    }

    game->perform_action("undo_bribe", name, target.get_name());
    game->cancel_last_action(target.get_name());
    game->next_turn();
    game->undo_bribe = true;
}

} // namespace coup
