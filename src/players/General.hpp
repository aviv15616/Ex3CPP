#pragma once

#include "Player.hpp"

namespace coup {

    class General : public Player {
    public:
        General(Game& game, const std::string& name);

        // החזרת שם התפקיד
        std::string role() const override;

        // יכול לחסום הפיכה (coup) שבוצעה על שחקן (כולל על עצמו) בתמורה ל־5 מטבעות
        void block_coup(Player& target);

        // כשהגנרל נעצר (arrest), הוא מקבל בחזרה את המטבע שנלקח ממנו
        void on_arrest();
    };

}
