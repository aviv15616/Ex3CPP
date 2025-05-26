#include "Judge.hpp"
#include "../core/Exceptions.hpp"
#include "../core/Game.hpp"
#include "Player.hpp"
#include <iostream>

namespace coup {

    Judge::Judge(Game& game, const std::string& name)
        : Player(game, name) {}

    std::string Judge::role() const {
        return "Judge";
    }

    void Judge::undo_bribe(Player& target) {
        const int bribe_cost = 4;

        if (!game->can_undo_action(target.get_name(), "bribe")) {
            throw UndoNotAllowed(role(), "undo_bribe");
        }

        if (target.coins() < bribe_cost) {
            throw NotEnoughCoinsException(bribe_cost, target.coins());
        }

        target.set_coins(target.coins() - bribe_cost);
        this->set_coins(this->coins() + bribe_cost);
        std::cout << "[Judge] " << name << " undoes bribe from " << target.get_name() << ", transferring 4 coins." << std::endl;
        game->cancel_last_action(target.get_name());
    }

    void Judge::on_sanction() {
        // במקרה ששחקן תקף את השופט, עליו לשלם עוד מטבע
        // נניח שהשחקן שקורא את sanction אחראי לבדוק את זה
        // ולכן לא נבצע פעולה כאן, אלא הלוגיקה תהיה אצל התוקף
        std::cout << "[Judge] " << name << " was sanctioned." << std::endl;
    }

} // namespace coup
