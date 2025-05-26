#include "General.hpp"
#include "../core/Exceptions.hpp"
#include "../core/Game.hpp"

namespace coup {

    General::General(Game& game, const std::string& name)
        : Player(game, name) {}

    std::string General::role() const {
        return "General";
    }

    void General::block_coup(Player& target) {
        if (coin_count < 5) {
            throw NotEnoughCoinsException(5, coin_count);
        }

        if (!game->is_coup_pending_on(target.get_name())) {
            throw InvalidActionException("No coup to block on this target.");
        }

        coin_count -= 5;
        game->cancel_coup();
    }

    void General::on_arrest() {
        coin_count += 1;
    }

} // namespace coup