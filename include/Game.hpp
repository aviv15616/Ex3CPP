// Anksilae@gmail.com

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include "Player.hpp"

namespace coup {

    class Game {
        friend class TestGame;

    private:
        // ===== Game State =====
        std::vector<std::shared_ptr<Player>> players_list;         // All players in the game
        size_t current_turn_index = 0;                             // Current player's turn index
        bool game_over = false;                                    // Game over flag
        int global_turn_counter = 0;                               // Number of turns passed

        // ===== State Logs =====
        std::string last_action;                                   // Last action description
        std::unordered_map<std::string, std::string> last_actions; // Player name → last action
        std::unordered_map<std::string, int> action_turn;          // Player name → turn number when last acted

        // ===== Arrest and Coup Logic =====
        std::string last_arrested;                                 // Last arrested target
        std::vector<std::pair<std::string, std::string>> coup_pending_list; // List of (attacker, target) awaiting resolution
        std::unordered_set<std::string> arrest_blocked_players;    // Players blocked from arrest

        // ===== Internal Utility =====
        void assert_game_active() const;                           // Ensure game isn't over

    public:
        Game();                                                    // Constructor

        // ===== Player Management =====
        std::shared_ptr<Player> add_player(const std::string &name, const std::string &role); // Add new player by name/role
        void add_player(const std::shared_ptr<Player> &p);                                   // Add existing player instance
        void remove_player(const std::string &victim);                                       // Remove player from game
        std::vector<std::shared_ptr<Player>> get_all_players_raw() const;                    // Return all player pointers

        // ===== Logging and Undo =====
        void log_action(const std::string &text);                  // Save descriptive string of last action
        std::string get_last_action() const { return last_action; } // Return last action string

        bool can_undo_action(const std::string &target_name, const std::string &expected_action) const; // Check if action matches
        bool can_still_undo(const std::string &player_name) const; // Check if undo is allowed by turn count
        void cancel_last_action(const std::string &player_name);   // Cancel player’s last action

        // Undo Flags (GUI support)
        bool undo_tax = false;
        bool undo_bribe = false;
        bool peek_disable = false;
        bool undo_coup = false;

        // ===== Turn Handling =====
        std::string turn() const;                                  // Get name of current turn player
        void next_turn();                                          // Advance to next player's turn
        void perform_action(const std::string &action_name, const std::string &by); // Record action without target
        void perform_action(const std::string &action_name, const std::string &by, const std::string &target_name); // With target
        const std::unordered_map<std::string, std::string>& get_last_actions() const; // Returns map of last actions
        std::shared_ptr<Player> get_player_by_name(const std::string &name) const;
        // ===== Game State Queries =====
        std::vector<std::string> players() const;                  // Return list of alive player names
        std::string winner() const;                                // Return winner's name if one remains
        bool is_game_over() const;                                 // Is game finished?
        void print_state() const;                                  // Print all player states
        void reset();                                              // Reset game to initial state

        // ===== Coup Logic =====
        void add_to_coup(const std::string &attacker, const std::string &target); // Register pending coup
        const std::vector<std::pair<std::string, std::string>> &get_coup_pending_list() const; // Get pending coups

        void cancel_coup(std::string target);                      // Cancel coup on given target
        bool is_coup_pending_on(const std::string &target) const;  // Check if target has pending coup
        void mark_coup_pending(const std::string &attacker, const std::string &target); // (unused currently)
        std::string get_coup_attacker(const std::string &target) const; // (unused currently)

        // ===== Arrest Logic =====
        void block_arrest_for(const std::string &name);            // Prevent player from arresting
        bool is_arrest_blocked(const std::string &name) const;     // Is arrest blocked for player?
        void set_last_arrest_target(const std::string &target);    // Register last arrest target
        bool arrested_same_target(const std::string &target) const;// Prevent repeat arrests
    };

}

