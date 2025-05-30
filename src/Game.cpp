// Game.cpp - Game Logic Implementation
// Anksilae@gmail.com

#include "Game.hpp"
#include "Exceptions.hpp"
#include <iostream>
#include <algorithm>
#include "Player.hpp"
#include "Governor.hpp"
#include "Spy.hpp"
#include "Judge.hpp"
#include "Baron.hpp"
#include "General.hpp"
#include "Merchant.hpp"

using namespace std;

namespace coup {

// ==============================
// Constructor & Initialization
// ==============================

/**
 * @brief Constructs a new Game and logs initialization.
 */
Game::Game() {
    std::cout << "[Game] Initialized new game.\n";
    this->log_action("[Game] Initialized new game.");
}

// ======================
// State & Validation
// ======================

/**
 * @brief Throws exception if game has ended.
 */
void Game::assert_game_active() const {
    if (game_over)
        throw GameAlreadyOverException();
}

/**
 * @brief Ends the game and logs the event.
 */
void Game::end_game() {
    if (game_over) {
        throw GameAlreadyOverException();
    }
    game_over = true;
    std::cout << "[Game] Game has ended.\n";
}

/**
 * @brief Checks if the game is over.
 * @return true if over, false otherwise.
 */
bool Game::is_game_over() const {
    return game_over;
}

// ======================
// Logging
// ======================

/**
 * @brief Logs the latest action for display and tracking.
 * @param text The action description.
 */
void Game::log_action(const std::string &text) {
    last_action = text;
}

// ======================
// Player Management
// ======================

/**
 * @brief Adds a new player to the game with the specified role.
 * @param name The name of the player.
 * @param role The role name (e.g., Spy, Judge).
 * @return Shared pointer to the created Player.
 * @throws DuplicatePlayerNameException if name already exists.
 * @throws InvalidActionException if role is invalid.
 */
std::shared_ptr<Player> Game::add_player(const std::string &name, const std::string &role) {
    assert_game_active();

    for (const auto& p : players_list) {
        if (p->get_name() == name) {
            throw DuplicatePlayerNameException(name);
        }
    }

    std::shared_ptr<Player> player;
    if      (role == "Governor") player = std::make_shared<Governor>(*this, name);
    else if (role == "Spy")      player = std::make_shared<Spy>(*this, name);
    else if (role == "Judge")    player = std::make_shared<Judge>(*this, name);
    else if (role == "Baron")    player = std::make_shared<Baron>(*this, name);
    else if (role == "General")  player = std::make_shared<General>(*this, name);
    else if (role == "Merchant") player = std::make_shared<Merchant>(*this, name);
    else throw InvalidActionException("Unknown role: " + role);

    players_list.push_back(player);
    player->set_active(true);
    std::cout << "[Game] Added player: " << name << " (" << role << ")\n";
    return player;
}

/**
 * @brief Adds an already-created player to the game (used for testing).
 * @param p Shared pointer to the player object.
 */
void Game::add_player(const std::shared_ptr<Player> &p) {
    assert_game_active();
    for (const auto& existing : players_list) {
        if (existing->get_name() == p->get_name()) {
            throw DuplicatePlayerNameException(p->get_name());
        }
    }
    players_list.push_back(p);
    std::cout << "[Game] Added player: " << p->get_name() << " (" << p->role() << ")\n";
}

/**
 * @brief Returns a list of names of all active players.
 */
std::vector<std::string> Game::players() const {
    std::vector<std::string> active;
    for (const auto &p : players_list)
        if (p->is_active()) active.push_back(p->get_name());
    return active;
}

/**
 * @brief Returns all players (active and eliminated).
 */
std::vector<std::shared_ptr<Player>> Game::get_all_players_raw() const {
    return players_list;
}

/**
 * @brief Returns the map of last actions per player.
 */
const std::unordered_map<std::string, std::string>& Game::get_last_actions() const {
    return last_actions;
}

/**
 * @brief Gets a player by name.
 * @throws PlayerNotFoundException if name not found.
 */
std::shared_ptr<Player> Game::get_player_by_name(const std::string &name) const {
    assert_game_active();
    for (auto &p : players_list)
        if (p->get_name() == name)
            return p;
    throw PlayerNotFoundException(name);
}

/**
 * @brief Marks a player as eliminated (inactive).
 * @param victim The player's name.
 * @throws PlayerNotFoundException if not found.
 */
void Game::remove_player(const std::string &victim) {
    assert_game_active();
    for (auto &p : players_list) {
        if (p->get_name() == victim) {
            p->set_active(false);
            std::cout << "[Eliminate] Player " << victim << " has been eliminated(unless undone by a general).\n";
            this->log_action("[Eliminate] Player " + victim + " has been eliminated(unless undone by a general).\n");
            return;
        }
    }
    throw PlayerNotFoundException(victim);
}

// ======================
// Turn Logic
// ======================

/**
 * @brief Gets the name of the current player in turn.
 */
std::string Game::turn() const {
    if (players_list.empty())
        throw InvalidActionException("No players in game.");
    return players_list.at(current_turn_index)->get_name();
}

/**
 * @brief Advances the game to the next active player's turn.
 */
void Game::next_turn() {
    assert_game_active();
    global_turn_counter++;

    std::string prev = turn();
    std::shared_ptr<Player> prev_player = get_player_by_name(prev);
    prev_player->unsanction();

    if (players().size() == 1) {
        std::cout << "[Game] Winner is: " << players().front() << std::endl;
        log_action("[Game] Winner is: " + players().front());
        game_over = true;
        return;
    }

    size_t n = players_list.size();
    do {
        current_turn_index = (current_turn_index + 1) % n;
    } while (!players_list[current_turn_index]->is_active());

    for (auto it = coup_pending_list.begin(); it != coup_pending_list.end(); ) {
        if (it->first == turn()) {
            it = coup_pending_list.erase(it);
        } else {
            ++it;
        }
    }

    prev_player->enable_arrest();

    std::cout << "[Turn] " << prev << " ended. " << turn() << " begins.\n";

    if (current_turn_index == players_list.size() - 1) {
        undo_tax = undo_bribe = peek_disable = undo_coup = false;
    }

    players_list[current_turn_index]->on_turn_start();
}

/**
 * @brief Gets the current turn index (for testing/debugging).
 */
int Game::get_current_turn_index() const {
    return current_turn_index;
}

/**
 * @brief Sets the current turn index.
 * @throws InvalidActionException if index is invalid.
 */
void Game::set_current_turn_index(int index) {
    if (index < 0 || index >= static_cast<int>(players_list.size())) {
        throw InvalidActionException("Invalid turn index: " + std::to_string(index));
    }
    current_turn_index = index;
}

/**
 * @brief Returns the name of the winner.
 * @throws GameNotOverException if more than one player is still active.
 */
std::string Game::winner() const {
    auto alive = players();
    if (alive.size() != 1)
        throw GameNotOverException();
    return alive.front();
}

// ======================
// Action Handling
// ======================

/**
 * @brief Performs a non-targeted action and logs it.
 */
void Game::perform_action(const std::string &action_name, const std::string &by) {
    perform_action(action_name, by, "");
}

/**
 * @brief Performs an action, optionally with a target, and logs it.
 */
void Game::perform_action(const std::string &action_name, const std::string &by, const std::string &target_name) {
    assert_game_active();
    last_actions[by] = action_name;
    action_turn[by] = global_turn_counter;

    auto actor = get_player_by_name(by);
    std::string log_line = "[" + action_name + "] performed by " + by + " (" + actor->role() + ")" +
                           " (Coins: " + std::to_string(actor->coins()) + ")";

    if (!target_name.empty()) {
        try {
            auto target = get_player_by_name(target_name);
            log_line += " on " + target_name + " (" + target->role() + ")" +
                        " (Coins: " + std::to_string(target->coins()) + ")";
        } catch (...) {
            log_line += " → " + target_name + " (Unknown)";
        }
    }

    std::cout << log_line << std::endl;
    this->log_action(log_line);
}

/**
 * @brief Cancels the last action of a player and logs it.
 */
void Game::cancel_last_action(const std::string &player_name) {
    last_actions.erase(player_name);

    std::string role;
    try {
        role = get_player_by_name(player_name)->role();
    } catch (...) {
        role = "Unknown";
    }

    std::string action_name = (role == "Judge") ? "bribe" : (role == "Governor") ? "tax" : "last action";
    log_action("[Undo] Cancelled " + action_name + " of: " + player_name);
}

/**
 * @brief Checks whether a specific action can be undone for a player.
 */
bool Game::can_undo_action(const std::string &target_name, const std::string &expected_action) const {
    auto it = last_actions.find(target_name);
    return (it != last_actions.end() && it->second == expected_action);
}

/**
 * @brief Checks whether a player's action is still eligible for undo based on turn count.
 */
bool Game::can_still_undo(const std::string &player_name) const {
    auto it = action_turn.find(player_name);
    if (it == action_turn.end()) return false;
    return (global_turn_counter - it->second) < static_cast<int>(players().size());
}

// ======================
// Arrest/Coup Control
// ======================

/**
 * @brief Disables arrest capability for a given player.
 */
void Game::block_arrest_for(const std::string &name) {
    assert_game_active();
    get_player_by_name(name)->disable_arrest();
    perform_action("block_arrest", turn(), name);
}

/**
 * @brief Checks if a player is blocked from arresting.
 */
bool Game::is_arrest_blocked(const std::string &name) const {
    return get_player_by_name(name)->is_arrest_disabled();
}

/**
 * @brief Sets the last player who was targeted for arrest.
 */
void Game::set_last_arrest_target(const std::string &target) {
    assert_game_active();
    last_arrested = target;
}

/**
 * @brief Checks if the last arrested player matches the given name.
 */
bool Game::arrested_same_target(const std::string &target) const {
    return last_arrested == target;
}

// ======================
// Coup System
// ======================

/**
 * @brief Adds a coup entry for an attacker and target.
 */
void Game::add_to_coup(const std::string &attacker, const std::string &target) {
    coup_pending_list.emplace_back(attacker, target);
}

/**
 * @brief Gets the list of all pending coups.
 */
const std::vector<std::pair<std::string, std::string>> &Game::get_coup_pending_list() const {
    return coup_pending_list;
}

/**
 * @brief Cancels a coup on the specified target and revives the player.
 * @throws InvalidActionException if no coup found.
 */
void Game::cancel_coup(const std::string& target) {
    assert_game_active();

    bool found = false;
    for (auto it = coup_pending_list.begin(); it != coup_pending_list.end(); ) {
        if (it->second == target) {
            it = coup_pending_list.erase(it);
            found = true;
        } else {
            ++it;
        }
    }

    if (found) {
        get_player_by_name(target)->set_active(true);
        log_action("[Coup] Coup on " + target + " has been cancelled.\n");
    } else {
        throw InvalidActionException("No pending coup on " + target);
    }
}

/**
 * @brief Checks if a coup is currently pending on a given player.
 */
bool Game::is_coup_pending_on(const std::string &target) const {
    assert_game_active();
    for (const auto &entry : coup_pending_list) {
        if (entry.second == target) {
            return true;
        }
    }
    return false;
}

// ======================
// Reset & Debug
// ======================

/**
 * @brief Resets the entire game state to start a new match.
 */
void Game::reset() {
    players_list.clear();
    current_turn_index = 0;
    game_over = false;
    coup_pending_list.clear();
    arrest_blocked_players.clear();
    last_arrested.clear();
    last_actions.clear();
    std::cout << "[Game] Reset complete.\n";
    log_action("[Game] Reset complete.\n");
}

/**
 * @brief Prints the current game state to the console (for debugging).
 */
[[maybe_unused]] void Game::print_state() const {
    std::cout << "\n===== Game State =====\n";
    std::cout << "Current turn: " << turn() << "\n";
    std::cout << "\nActive players:\n";
    for (const auto &p : players_list)
        if (p->is_active())
            std::cout << " - " << p->get_name() << " (" << p->role() << ") | Coins: " << p->coins() << "\n";

    std::cout << "\nEliminated players:\n";
    for (const auto &p : players_list)
        if (!p->is_active())
            std::cout << " - " << p->get_name() << " (" << p->role() << ") | Coins: " << p->coins() << "\n";
    std::cout << "======================\n" << std::endl;
}

} // namespace coup
