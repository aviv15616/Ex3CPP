#pragma once

#include "Player.hpp"

namespace coup {
    class Baron : public Player {
    public:
        Baron(Game& game, const std::string& name);
        
        std::string role() const override;

        // פעולה ייחודית של הברון בתורו
        void invest();

        // תגובה פסיבית כאשר מותקף ב-saction (פיצוי של 1)
        void on_sanction();
    };
}
