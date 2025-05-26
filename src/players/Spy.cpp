#include "Spy.hpp"
#include "../core/Game.hpp"
#include "../core/Exceptions.hpp"
#include <iostream>

namespace coup {

    Spy::Spy(Game& game, const std::string& name)
        : Player(game, name) {}

    std::string Spy::role() const {
        return "Spy";
    }

    void Spy::peek(const Player& target) const {
        std::cout << "[Spy] " << name << " peeked: " << target.get_name() << " has " << target.coins() << " coins." << std::endl;
    }

    void Spy::disable_arrest(Player& target) {
        if (!target.is_active()) {
            throw PlayerAlreadyDeadException(target.get_name());
        }

        game->block_arrest_for(target.get_name());
        std::cout << "[Spy] " << name << " disabled arrest for " << target.get_name() << std::endl;
    }

} // namespace coup
