#include "Game.hpp"
#include "../core/Exceptions.hpp"
#include <iostream>

using namespace std;

namespace coup {

    Game::Game() {
        std::cout << "[Game] Initialized new game.\n";
    }

    void Game::add_player(const shared_ptr<Player>& player) {
        if (game_over) {
            throw GameAlreadyOverException();
        }

        for (const auto& p : players_list) {
            if (p->get_name() == player->get_name()) {
                throw DuplicatePlayerNameException(player->get_name());
            }
        }

        players_list.push_back(player);
        std::cout << "[Game] Added player: " << player->get_name() << " (" << player->role() << ")\n";
    }

    string Game::turn() const {
        if (players_list.empty()) {
            throw InvalidActionException("No players in game.");
        }
        return players_list.at(current_turn_index)->get_name();
    }

    void Game::perform_action(const std::string& action_name, const std::string& by) {
        if (turn() != by) {
            throw InvalidActionException("It's not this player's turn.");
        }
        last_actions[by] = action_name;
        std::cout << "[Action] " << by << " performs action: " << action_name << std::endl;
    }

    bool Game::can_undo_action(const std::string& target_name, const std::string& expected_action) const {
        auto it = last_actions.find(target_name);
        if (it == last_actions.end()) return false;
        return it->second == expected_action;
    }

    void Game::cancel_last_action(const std::string& player_name) {
        last_actions.erase(player_name);
        std::cout << "[Undo] Cancelled last action of: " << player_name << std::endl;
    }

    vector<string> Game::players() const {
        vector<string> active;
        for (const auto& p : players_list) {
            if (p->is_active()) {
                active.push_back(p->get_name());
            }
        }
        return active;
    }

    string Game::winner() const {
        vector<string> alive = players();
        if (alive.size() != 1) {
            throw GameNotOverException();
        }
        std::cout << "[Game] Winner is: " << alive.front() << std::endl;
        return alive.front();
    }

    void Game::next_turn() {
        std::string prev = turn();

        size_t n = players_list.size();
        do {
            current_turn_index = (current_turn_index + 1) % n;
        } while (!players_list[current_turn_index]->is_active());

        if (last_coup_pending.has_value() && last_coup_pending->first == prev) {
            last_coup_pending.reset();
        }

        last_actions.erase(turn());
        arrest_blocked_players.erase(prev);

        std::cout << "[Turn] " << prev << " ended. " << turn() << " begins.\n";

        players_list[current_turn_index]->on_turn_start();
    }

    void Game::remove_player(const string& name) {
        for (auto& p : players_list) {
            if (p->get_name() == name) {
                p->set_active(false);
                std::cout << "[Eliminate] Player " << name << " has been eliminated.\n";
                return;
            }
        }
        throw PlayerNotFoundException(name);
    }

    shared_ptr<Player> Game::get_player_by_name(const string& name) {
        for (auto& p : players_list) {
            if (p->get_name() == name && p->is_active()) {
                return p;
            }
        }
        throw PlayerNotFoundException(name);
    }

    void Game::block_arrest_for(const string& name) {
        arrest_blocked_players.insert(name);
        std::cout << "[Spy] Arrest blocked for: " << name << std::endl;
    }

    bool Game::is_arrest_blocked(const string& name) const {
        return arrest_blocked_players.find(name) != arrest_blocked_players.end();
    }

    void Game::set_last_arrest_target(const string& attacker, const string& target) {
        last_arrest_target[attacker] = target;
        std::cout << "[Arrest] " << attacker << " arrested " << target << std::endl;
    }

    bool Game::arrested_same_target_last_turn(const string& attacker, const string& target) const {
        auto it = last_arrest_target.find(attacker);
        return it != last_arrest_target.end() && it->second == target;
    }

    void Game::mark_coup_pending(const std::string& attacker, const std::string& target) {
        last_coup_pending = std::make_pair(attacker, target);
        std::cout << "[Coup] Marked coup from " << attacker << " to " << target << std::endl;
    }

    bool Game::is_coup_pending_on(const std::string& target) const {
        return last_coup_pending.has_value() && last_coup_pending->second == target;
    }

    std::string Game::get_coup_attacker(const std::string& target) const {
        if (!is_coup_pending_on(target)) {
            throw InvalidActionException("No coup pending on this target.");
        }
        return last_coup_pending->first;
    }

    void Game::cancel_coup() {
        std::cout << "[Coup] Coup cancelled.\n";
        last_coup_pending.reset();
    }

    void Game::print_state() const {
        using std::cout;
        using std::endl;

        cout << "\n===== Game State =====" << endl;
        cout << "Current turn: " << turn() << endl;

        cout << "\nActive players:" << endl;
        for (const auto& p : players_list) {
            if (p->is_active()) {
                cout << " - " << p->get_name() << " (" << p->role() << ")"
                     << " | Coins: " << p->coins() << endl;
            }
        }

        cout << "\nEliminated players:" << endl;
        for (const auto& p : players_list) {
            if (!p->is_active()) {
                cout << " - " << p->get_name() << " (" << p->role() << ")"
                     << " | Coins: " << p->coins() << endl;
            }
        }

        if (last_coup_pending.has_value()) {
            cout << "\nPending coup: " << last_coup_pending->first
                 << " → " << last_coup_pending->second << endl;
        }

        cout << "======================\n" << endl;
    }

    void Game::reset() {
        players_list.clear();
        current_turn_index = 0;
        game_over = false;
        last_coup_pending.reset();
        arrest_blocked_players.clear();
        last_arrest_target.clear();
        last_actions.clear();
        std::cout << "[Game] Reset complete.\n";
    }

    bool Game::is_game_over() const {
        return game_over;
    }

} // namespace coup
