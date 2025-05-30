// Anksilae@gmail.com

#include "General.hpp"
#include "Exceptions.hpp"
#include "Game.hpp"

namespace coup {

/**
 * @brief Constructs a General role player and registers it to the game.
 * 
 * @param game Reference to the game instance.
 * @param name Name of the player.
 */
General::General(Game& game, const std::string& name)
    : Player(game, name) {}

/**
 * @brief Returns the role name of this player ("General").
 */
std::string General::role() const {
    return "General";
}

/**
 * @brief Allows the General to undo a coup on a target player.
 * 
 * Costs 5 coins. Only one undo_coup is allowed per round.
 * 
 * @param target The player whose coup is to be undone.
 * @throws NotEnoughCoinsException if the player has less than 5 coins.
 * @throws InvalidActionException if no coup is pending or already undone this round.
 */
void General::undo_coup(Player& target) {
    if (coin_count < 5) {
        throw NotEnoughCoinsException(5, coin_count);
    }

    if (!game->is_coup_pending_on(target.get_name())) {
        throw InvalidActionException("No coup to block on this target.");
    }

    if (game->undo_coup == true) {
        throw InvalidActionException("Coup already undone this round.");
    }

    coin_count -= 5;
    game->cancel_coup(target.get_name());
    game->undo_coup = true;
}

/**
 * @brief Called when the General is arrested — receives 1 coin as compensation.
 */
void General::on_arrest() {
    set_coins(coins() + 1); // פיצוי של מטבע אחד על כל מעצר
}

} // namespace coup
