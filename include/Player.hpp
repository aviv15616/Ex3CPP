// Anksilae@gmail.com

#pragma once

#include <string>
#include <memory>

namespace coup {

    class Game; // Forward declaration to avoid including Game.hpp

    class Player {
    protected:
        std::string name;
        Game* game;
        int coin_count = 0;
        bool active = true;
        bool under_arrest = false;
        bool under_sanction = false;
        bool arrest_disabled =false;

        
        

    public:
        Player(Game& game_ref, const std::string& name);
        virtual ~Player() = default;

        // ===== State and Info Accessors =====
        const std::string& get_name() const;          // Returns the player's name
        int coins() const;                            // Returns the player's coin count
        void set_coins(int amount);                   // Sets the player's coin count
        bool is_active() const;                       // Returns whether the player is alive
        void set_active(bool status);                 // Sets the active (alive/dead) status
      
        // ===== Abstract Methods =====
        virtual std::string role() const = 0;         // Returns the role name (to be overridden)

        // ===== Basic Actions =====
        virtual void gather();                        // +1 coin
        void skip_turn();
        virtual void tax();                           // +2 coins
        void bribe();                                 // Pay 4 coins to bribe

        // ===== Targeted Actions =====
        void arrest(Player& target);                  // Arrest another player and steal coins
        void sanction(Player& target);                // Apply sanction to another player
        void coup(Player& target);                    // Eliminate another player (coup)
        bool is_sanctioned() const ; // Check if player is sanctioned
        bool is_arrest_disabled() const ; // Check if arrest is disabled
        void enable_arrest() ; // Enable arrest for this player
        void disable_arrest(); // Disable arrest for this player
        void ensure_coup_required(); // Throws if player has ≥10 coins and hasn't performed a coup

        // ===== Undoable Actions (Override in roles) =====
        virtual void undo_tax(Player& target);        // Undo a tax action (Governor only)
        virtual void undo_bribe(Player& target);      // Undo a bribe action (Judge only)

        // ===== Turn Start Hook =====
        virtual void on_turn_start();                 // Called at start of player's turn

        // ===== Passive Reactions =====
        virtual void on_arrest();                     // Called when player is arrested
        void unsanction();                            // Clears sanction status
        virtual void on_sanction();                   // Called when player receives sanction
    };

} // namespace coup
