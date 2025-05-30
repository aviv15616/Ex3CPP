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

/**
 * @brief Core game logic for managing players, turns, actions, coup system, and undo logic.
 * 
 * This class is responsible for tracking game state, validating actions,
 * managing turn order, and logging activity for UI integration and role effects.
 */
class Game {
    friend class TestGame;

private:
    // ===== Game State =====
    std::vector<std::shared_ptr<Player>> players_list; ///< All players in the game
    size_t current_turn_index = 0;                     ///< Current player's index
    bool game_over = false;                            ///< Game over flag
    int global_turn_counter = 0;                       ///< Number of turns passed

    // ===== State Logs =====
    std::string last_action;                                   ///< Description of last action
    std::unordered_map<std::string, std::string> last_actions; ///< Player → last action performed
    std::unordered_map<std::string, int> action_turn;          ///< Player → turn number of last action

    // ===== Arrest and Coup Logic =====
    std::string last_arrested;                                          ///< Last arrested target name
    std::vector<std::pair<std::string, std::string>> coup_pending_list; ///< List of pending coups (attacker → target)
    std::unordered_set<std::string> arrest_blocked_players;             ///< Players blocked from using arrest

    // ===== Internal Validation =====
    void assert_game_active() const; ///< Throws if game is over

public:
    // ===== Constructor =====
    Game();

    // ===== Player Management =====

    /**
     * @brief Add a new player by name and role.
     */
    std::shared_ptr<Player> add_player(const std::string &name, const std::string &role);

    /**
     * @brief Add an existing Player instance (used in testing).
     */
    void add_player(const std::shared_ptr<Player> &p);

    /**
     * @brief Remove a player from the game (sets them inactive).
     */
    void remove_player(const std::string &victim);

    /**
     * @brief Return list of raw Player pointers (alive and dead).
     */
    std::vector<std::shared_ptr<Player>> get_all_players_raw() const;

    // ===== Logging and Undo =====

    /**
     * @brief Save a descriptive string representing the last action.
     */
    void log_action(const std::string &text);

    /**
     * @brief Get string description of last action.
     */
    std::string get_last_action() const { return last_action; }

    /**
     * @brief Check if last action of a player matches a specific type (e.g. "tax").
     */
    bool can_undo_action(const std::string &target_name, const std::string &expected_action) const;

    /**
     * @brief Returns true if a player's last action can still be undone based on turn distance.
     */
    bool can_still_undo(const std::string &player_name) const;

    /**
     * @brief Cancels the last recorded action for a player.
     */
    void cancel_last_action(const std::string &player_name);

    // Undo Flags (used by GUI)
    bool undo_tax = false;
    bool undo_bribe = false;
    bool peek_disable = false;
    bool undo_coup = false;

    // ===== Turn Handling =====

    /**
     * @brief Get the name of the player whose turn it is.
     */
    std::string turn() const;

    /**
     * @brief Advance to the next active player's turn.
     */
    void next_turn();

    /**
     * @brief Get current turn index.
     */
    int get_current_turn_index() const;

    /**
     * @brief Set turn index manually (used for tests).
     */
    void set_current_turn_index(int index);

    /**
     * @brief Record an action performed by a player (no target).
     */
    void perform_action(const std::string &action_name, const std::string &by);

    /**
     * @brief Record an action performed by a player on a target.
     */
    void perform_action(const std::string &action_name, const std::string &by, const std::string &target_name);

    /**
     * @brief Get the full map of players' last actions.
     */
    const std::unordered_map<std::string, std::string> &get_last_actions() const;

    /**
     * @brief Retrieve player instance by name.
     */
    std::shared_ptr<Player> get_player_by_name(const std::string &name) const;

    // ===== Game State Queries =====

    /**
     * @brief Get names of all active players.
     */
    std::vector<std::string> players() const;

    /**
     * @brief Return the winner's name if only one remains.
     * @throws GameNotOverException if game is not over.
     */
    std::string winner() const;

    /**
     * @brief Check if the game is over.
     */
    bool is_game_over() const;

    /**
     * @brief Print current state of all players (debugging).
     */
    void print_state() const;

    /**
     * @brief Reset the game to initial state.
     */
    void reset();

    /**
     * @brief End the game immediately (for testing purposes).
     */
    void end_game();

    // ===== Coup Logic =====

    /**
     * @brief Add a pending coup entry (attacker → target).
     */
    void add_to_coup(const std::string &attacker, const std::string &target);

    /**
     * @brief Get list of pending coup pairs.
     */
    const std::vector<std::pair<std::string, std::string>> &get_coup_pending_list() const;

    /**
     * @brief Cancel a pending coup on a specific target.
     */
    void cancel_coup(const std::string &target);

    /**
     * @brief Check if there is a pending coup on the given target.
     */
    bool is_coup_pending_on(const std::string &target) const;

    // (Currently unused internal helpers)
    void mark_coup_pending(const std::string &attacker, const std::string &target);
    std::string get_coup_attacker(const std::string &target) const;

    // ===== Arrest Logic =====

    /**
     * @brief Prevent a player from using arrest this round.
     */
    void block_arrest_for(const std::string &name);

    /**
     * @brief Check if arrest is blocked for a player.
     */
    bool is_arrest_blocked(const std::string &name) const;

    /**
     * @brief Record the name of the last arrested player.
     */
    void set_last_arrest_target(const std::string &target);

    /**
     * @brief Returns true if same target was arrested in last turn.
     */
    bool arrested_same_target(const std::string &target) const;
};

} // namespace coup
