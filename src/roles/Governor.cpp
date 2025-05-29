// Anksilae@gmail.com

#include "Governor.hpp"
#include "Game.hpp"
#include "Exceptions.hpp"
#include <iostream>

namespace coup {

    Governor::Governor(Game& game, const std::string& name)
        : Player(game, name) {}

    std::string Governor::role() const {
        return "Governor";
    }

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
        std::cout << "[Governor] " << name << " collects 3 coins via tax. Total: " << coin_count << std::endl;
        game->next_turn();
    }

    void Governor::undo_tax(Player& target) {
        if (!game->can_undo_action(target.get_name(), "tax")) {
            throw UndoNotAllowed(role(), "undo_tax");
        }
        if( game->undo_tax) {
            throw InvalidActionException("Tax already undone this round.");
        }
        if (target.get_name() == name) {
            throw CannotTargetYourselfException("undo tax");        }
        
        int undo_amount = 2;
        if(target.role() == "Governor"){
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
