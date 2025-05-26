#include "Merchant.hpp"
#include "../core/Exceptions.hpp"
#include "../core/Game.hpp"
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

    // במקרה של arrest - משלם 2 לקופה (לא מועבר ל-attacker)
    void Merchant::on_arrest() {
        if (coins() < 2) {
            throw NotEnoughCoinsException(2, coins());
        }
        set_coins(coins() - 2);
        std::cout << "[Merchant] " << name << " paid 2 coins to pot after arrest. Remaining: " << coins() << std::endl;
    }

} // namespace coup
