#include "General.hpp"
#include "Exceptions.hpp"
#include "Game.hpp"

namespace coup {

    General::General(Game& game, const std::string& name)
        : Player(game, name) {}

    std::string General::role() const {
        return "General";
    }

    void General::undo_coup(Player& target) {
        if (coin_count < 5) {
            throw NotEnoughCoinsException(5, coin_count);
        }

        if (!game->is_coup_pending_on(target.get_name())) {
            throw InvalidActionException("No coup to block on this target.");
        }
        if(game->undo_coup==true){
            throw InvalidActionException("Coup already undone this round.");
        }

        coin_count -= 5;
        game->cancel_coup(target.get_name());
        game->undo_coup=true;
    }

    void General::on_arrest() {
        set_coins(coins() + 1); // פיצוי של מטבע אחד על כל מעצר
    }

} // namespace coup