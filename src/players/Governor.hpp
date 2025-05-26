#pragma once

#include "Player.hpp"

namespace coup {

    class Governor : public Player {
    public:
        Governor(Game& game, const std::string& name);

        // החזרת שם התפקיד
        std::string role() const override;

        // פעולה מיוחדת - לוקח 3 מטבעות במקום 2
        void tax() override;

        // יכולת לבטל פעולה של tax לשחקנים אחרים
        void undo_tax(Player& target);
    };

}
