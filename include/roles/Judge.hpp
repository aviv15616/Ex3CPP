// Anksilae@gmail.com

#pragma once

#include "Player.hpp"

namespace coup {

    class Judge : public Player {
    public:
        Judge(Game& game, const std::string& name);

        // החזרת שם התפקיד
        std::string role() const override;

        // יכול לבטל פעולה של שוחד (bribe) ולגבות את העלות מהשחקן שביצע
        void undo_bribe(Player& target) override;

      
    };

}
