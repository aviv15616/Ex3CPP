// Anksilae@gmail.com

#pragma once

#include "Player.hpp"


namespace coup {

    class Spy : public Player {
    private:
        int peeked_coins = 0; // מטבעות שנצפו, לא בשימוש כרגע
        std::string peeked_role; // תפקיד שנצפה, לא בשימוש כרגע
        std::string peeked_name; // שם השחקן שנצפה, לא בשימוש כרגע
    public:
        Spy(Game& game, const std::string& name);

        // החזרת שם התפקיד
        std::string role() const override;

    

        // חוסם מטרה מלעשות arrest בתור הבא (לא נחשב כתור)
        void peek_and_disable(Player& target);
    };

}
