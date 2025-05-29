// Anksilae@gmail.com

#include "Merchant.hpp"
#include "Exceptions.hpp"
#include "Game.hpp"
#include <iostream>

namespace coup {

    Merchant::Merchant(Game& game, const std::string& name)
        : Player(game, name) {}

    std::string Merchant::role() const {
        return "Merchant";
    }

    // בונוס בתחילת תור: אם יש לפחות 3 מטבעות, מקבל מטבע נוסף
    void Merchant::on_turn_start() {
        if (coins() >= 3) {
            set_coins(coins() + 1);
            std::cout << "[Merchant] " << name << " gained 1 bonus coin at start of turn. Total: " << coins() << std::endl;
        }
    }



} // namespace coup
