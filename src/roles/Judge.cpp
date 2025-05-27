#include "Judge.hpp"
#include "Exceptions.hpp"
#include "Game.hpp"
#include "Player.hpp"
#include <iostream>

namespace coup {

    Judge::Judge(Game& game, const std::string& name)
        : Player(game, name) {}

    std::string Judge::role() const {
        return "Judge";
    }

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

    void Judge::on_sanction() {
        // במקרה ששחקן תקף את השופט, עליו לשלם עוד מטבע
        // נניח שהשחקן שקורא את sanction אחראי לבדוק את זה
        // ולכן לא נבצע פעולה כאן, אלא הלוגיקה תהיה אצל התוקף
        std::cout << "[Judge] " << name << " was sanctioned." << std::endl;
    }

} // namespace coup
