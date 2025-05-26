#include "Governor.hpp"
#include "../core/Game.hpp"
#include "../core/Exceptions.hpp"
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
        ensure_coup_required();
        game->perform_action("tax", name);
        coin_count += 3;
        std::cout << "[Governor] " << name << " collects 3 coins via tax. Total: " << coin_count << std::endl;
        game->next_turn();
    }

    void Governor::undo_tax(Player& target) {
        if (!game->can_undo_action(target.get_name(), "tax")) {
            throw UndoNotAllowed(role(), "undo_tax");
        }

        const int undo_amount = 2;

        if (target.coins() < undo_amount) {
            throw NotEnoughCoinsException(undo_amount, target.coins());
        }

        target.set_coins(target.coins() - undo_amount);
        this->set_coins(this->coins() + undo_amount);
        std::cout << "[Governor] " << name << " undoes tax from " << target.get_name() << ", returning 2 coins." << std::endl;
        game->cancel_last_action(target.get_name());
    }

} // namespace coup
