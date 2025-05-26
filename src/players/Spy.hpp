#pragma once

#include "Player.hpp"

namespace coup {

    class Spy : public Player {
    public:
        Spy(Game& game, const std::string& name);

        // החזרת שם התפקיד
        std::string role() const override;

        // מדפיס את כמות המטבעות של שחקן אחר
        void peek(const Player& target) const;

        // חוסם מטרה מלעשות arrest בתור הבא (לא נחשב כתור)
        void disable_arrest(Player& target);
    };

}
