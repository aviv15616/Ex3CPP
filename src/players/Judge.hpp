#pragma once

#include "Player.hpp"

namespace coup {

    class Judge : public Player {
    public:
        Judge(Game& game, const std::string& name);

        // החזרת שם התפקיד
        std::string role() const override;

        // יכול לבטל פעולה של שוחד (bribe) ולגבות את העלות מהשחקן שביצע
        void undo_bribe(Player& target);

        // כאשר השופט מותקף ב-sanction, התוקף צריך לשלם עוד מטבע (יש לממש מהצד של התוקף)
        void on_sanction();
    };

}
