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

Game::Game() {
    std::cout << "[Game] Initialized new game.\n";
    this->log_action("[Game] Initialized new game.");
}

// ======================
// State & Validation
// ======================

void Game::assert_game_active() const {
    if (game_over)
        throw GameAlreadyOverException();
}

bool Game::is_game_over() const {
    return game_over;
}

// ======================
// Logging
// ======================

void Game::log_action(const std::string &text) {
    last_action = text;
}

// ======================
// Player Management
// ======================

std::shared_ptr<Player> Game::add_player(const std::string &name, const std::string &role) {
    assert_game_active();

    if (std::any_of(players_list.begin(), players_list.end(), [&](const auto& p) { return p->get_name() == name; })) {
        throw DuplicatePlayerNameException(name);
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

void Game::add_player(const std::shared_ptr<Player> &p) {
    assert_game_active();
    if (std::any_of(players_list.begin(), players_list.end(), [&](const auto& existing) { return existing->get_name() == p->get_name(); })) {
        throw DuplicatePlayerNameException(p->get_name());
    }
    players_list.push_back(p);
    std::cout << "[Game] Added player: " << p->get_name() << " (" << p->role() << ")\n";
}

std::vector<std::string> Game::players() const {
    std::vector<std::string> active;
    for (const auto &p : players_list)
        if (p->is_active()) active.push_back(p->get_name());
    return active;
}

std::vector<std::shared_ptr<Player>> Game::get_all_players_raw() const {
    return players_list;
}
const std::unordered_map<std::string, std::string>& Game::get_last_actions() const {
    return last_actions;
}

std::shared_ptr<Player> Game::get_player_by_name(const std::string &name) {
    assert_game_active();
    for (auto &p : players_list)
        if (p->get_name() == name)
            return p;
    throw PlayerNotFoundException(name);
}

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

std::string Game::turn() const {
    if (players_list.empty())
        throw InvalidActionException("No players in game.");
    return players_list.at(current_turn_index)->get_name();
}

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

    coup_pending_list.erase(
        std::remove_if(
            coup_pending_list.begin(), coup_pending_list.end(),
            [&](const auto &entry) { return entry.first == turn(); }),
        coup_pending_list.end());

    arrest_blocked_players.erase(prev);

    std::cout << "[Turn] " << prev << " ended. " << turn() << " begins.\n";

    if (current_turn_index == players_list.size() - 1) {
        undo_tax = undo_bribe = peek_disable = undo_coup = false;
    }
}

std::string Game::winner() const {
    auto alive = players();
    if (alive.size() != 1)
        throw GameNotOverException();
    return alive.front();
}

// ======================
// Action Handling
// ======================

void Game::perform_action(const std::string &action_name, const std::string &by) {
    perform_action(action_name, by, "");
}

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

bool Game::can_undo_action(const std::string &target_name, const std::string &expected_action) const {
    auto it = last_actions.find(target_name);
    return (it != last_actions.end() && it->second == expected_action);
}

bool Game::can_still_undo(const std::string &player_name) const {
    auto it = action_turn.find(player_name);
    if (it == action_turn.end()) return false;
    return (global_turn_counter - it->second) < static_cast<int>(players().size());
}

// ======================
// Arrest/Coup Control
// ======================

void Game::block_arrest_for(const std::string &name) {
    assert_game_active();
    arrest_blocked_players.insert(name);
    perform_action("block_arrest", turn(), name);
}

bool Game::is_arrest_blocked(const std::string &name) const {
    return arrest_blocked_players.find(name) != arrest_blocked_players.end();
}

void Game::set_last_arrest_target(const std::string &target) {
    assert_game_active();
    last_arrested = target;
}

bool Game::arrested_same_target(const std::string &target) const {
    return last_arrested == target;
}

// ======================
// Coup System
// ======================

void Game::add_to_coup(const std::string &attacker, const std::string &target) {
    coup_pending_list.emplace_back(attacker, target);
}

const std::vector<std::pair<std::string, std::string>> &Game::get_coup_pending_list() const {
    return coup_pending_list;
}

void Game::cancel_coup(std::string target) {
    assert_game_active();

    auto it = std::remove_if(coup_pending_list.begin(), coup_pending_list.end(),
                             [&](const auto &entry) { return entry.second == target; });

    if (it != coup_pending_list.end()) {
        coup_pending_list.erase(it, coup_pending_list.end());
        get_player_by_name(target)->set_active(true);
        log_action("[Coup] Coup on " + target + " has been cancelled.\n");
    } else {
        throw InvalidActionException("No pending coup on " + target);
    }
}

bool Game::is_coup_pending_on(const std::string &target) const {
    assert_game_active();
    return std::any_of(coup_pending_list.begin(), coup_pending_list.end(),
                       [&](const auto &entry) { return entry.second == target; });
}

// ======================
// Reset & Debug
// ======================

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

void Game::print_state() const {
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
