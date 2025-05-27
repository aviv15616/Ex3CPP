#pragma once

#include "Player.hpp"

namespace coup {

    class Merchant : public Player {
    public:
        Merchant(Game& game, const std::string& name);

        std::string role() const override;

        // תוספת בתחילת תור אם יש 3 מטבעות לפחות
        void on_turn_start() override;

        // כאשר נעשה עליו arrest - משלם 2 לקופה
        void on_arrest() override;
    };

}
