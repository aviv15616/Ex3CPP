#pragma once

#include <string>
#include <memory>

namespace coup {
    class Game; // Forward declaration

    class Player {
    protected:
        std::string name;
        Game* game;
        int coin_count = 0;
        bool active = true;
        bool under_arrest=false;
        bool under_sanction = false;  


        
        void ensure_coup_required();

    public:
        Player(Game& game_ref, const std::string& name);
        virtual ~Player() = default;

        // Accessors
        const std::string& get_name() const;
        int coins() const;
        void set_coins(int amount);
        bool is_active() const;
        void set_active(bool status);
        bool has_available_actions() const;

        // Abstract - to be overridden by subclasses
        virtual std::string role() const = 0;

        // Basic actions
        virtual void gather();
        virtual void tax();
        void bribe();
        void arrest(Player& target);
        void sanction(Player& target);
        void coup(Player& target);
        virtual void undo_tax(Player& target);
        virtual void undo_bribe(Player& target);
        virtual void on_turn_start();

        // Passive reactions
        virtual void on_arrest();   // called when this player is arrested
        void unsanction();
        virtual void on_sanction(); // called when this player is sanctioned
    };
}
