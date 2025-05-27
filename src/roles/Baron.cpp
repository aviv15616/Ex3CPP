#include "Baron.hpp"
#include "Exceptions.hpp"
#include "Game.hpp"
#include <iostream>

namespace coup {

    Baron::Baron(Game& game, const std::string& name)
        : Player(game, name) {}

    std::string Baron::role() const {
        return "Baron";
    }

    void Baron::invest() {
        if (game->turn() != name) {
            throw NotYourTurnException();
        }
        ensure_coup_required();

        if (coin_count < 3) {
            throw NotEnoughCoinsException(3, coin_count); 
        }

       
        coin_count += 3;
        game->perform_action("invest", name);
        std::cout << "[Baron] " << name << " invested 3 coins and gained 6. Total: " << coin_count << std::endl;
        game->next_turn();
    }

    void Baron::on_sanction() {
        set_coins(coins() + 1);
        under_sanction = true;
        std::cout << "[Baron] " << name << " received 1 coin compensation after sanction. Total: " << coin_count << std::endl;
    }

} // namespace coup
