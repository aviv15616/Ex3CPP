// Anksilae@gmail.com

#include "Governor.hpp"
#include "Game.hpp"
#include "Exceptions.hpp"
#include <iostream>

namespace coup {

/**
 * @brief Constructs a Governor role player and registers it to the game.
 * 
 * @param game Reference to the game instance.
 * @param name Name of the player.
 */
Governor::Governor(Game& game, const std::string& name)
    : Player(game, name) {}

/**
 * @brief Returns the role name of this player ("Governor").
 */
std::string Governor::role() const {
    return "Governor";
}

/**
 * @brief Performs the Governor's version of tax (3 coins).
 * 
 * @throws NotYourTurnException if not in turn.
 * @throws InvalidActionException if under sanction or coup is required.
 */
void Governor::tax() {
    if (game->turn() != name) {
        throw NotYourTurnException();
    }
    if (under_sanction) {
        throw InvalidActionException("You are under sanction and cannot use Gather/Tax this turn.");
    }
    ensure_coup_required();

    coin_count += 3;
    game->perform_action("tax", name);
    game->next_turn();
}

/**
 * @brief Undoes a tax action performed by another player.
 * 
 * If successful, the taxed player loses their tax income.
 * 
 * @param target The player whose tax will be undone.
 * @throws UndoNotAllowed if the target's last action wasn't tax.
 * @throws InvalidActionException if already undone this round or insufficient coins.
 * @throws CannotTargetYourselfException if trying to undo own tax.
 */
void Governor::undo_tax(Player& target) {
    if (!game->can_undo_action(target.get_name(), "tax")) {
        throw UndoNotAllowed(role(), "undo_tax");
    }
    if (game->undo_tax) {
        throw InvalidActionException("Tax already undone this round.");
    }
    if (target.get_name() == name) {
        throw CannotTargetYourselfException("undo tax");
    }

    int undo_amount = 2;
    if (target.role() == "Governor") {
        undo_amount = 3;
    }

    if (target.coins() < undo_amount) {
        throw NotEnoughCoinsException(undo_amount, target.coins());
    }

    target.set_coins(target.coins() - undo_amount);
    std::cout << "[Governor] " << name << " undoes tax from " << target.get_name()
              << ", returning " << undo_amount << " coins." << std::endl;

    game->cancel_last_action(target.get_name());
    game->undo_tax = true;
}

} // namespace coup
