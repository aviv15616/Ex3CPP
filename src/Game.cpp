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

namespace coup
{

    Game::Game()
    {
        std::cout << "[Game] Initialized new game.\n";
        this->log_action("[Game] Initialized new game.");
    }

    void Game::assert_game_active() const
    {
        if (game_over)
            throw GameAlreadyOverException();
    }
    bool Game::is_name_taken(const std::string &name) const
    {
        for (const auto &p : players_list)
        {
            if (p->get_name() == name)
                return true;
        }
        return false;
    }
    void Game::log_action(const std::string &text)
    {
        last_action = text;
        std::cout << text << std::endl;
    }

    std::shared_ptr<Player> Game::add_player(const std::string &name, const std::string &role)
    {
        assert_game_active();

        for (const auto &p : players_list)
        {
            if (p->get_name() == name)
            {
                throw DuplicatePlayerNameException(name);
            }
        }

        std::shared_ptr<Player> player;

        if (role == "Governor")
            player = std::make_shared<Governor>(*this, name);
        else if (role == "Spy")
            player = std::make_shared<Spy>(*this, name);
        else if (role == "Judge")
            player = std::make_shared<Judge>(*this, name);
        else if (role == "Baron")
            player = std::make_shared<Baron>(*this, name);
        else if (role == "General")
            player = std::make_shared<General>(*this, name);
        else if (role == "Merchant")
            player = std::make_shared<Merchant>(*this, name);
        else
            throw InvalidActionException("Unknown role: " + role);

        players_list.push_back(player);
        player->set_active(true);
        std::cout << "[Game] Added player: " << name << " (" << role << ")\n";
        return player;
    }
    void Game:: add_to_coup(const std::string &attacker, const std::string &target)
    {
        coup_pending_list.emplace_back(attacker, target);
        std::cout << "[Coup] " << attacker << " marked for coup against " << target << std::endl;
    }
const std::vector<std::pair<std::string, std::string>>& Game::get_coup_pending_list() const
{
    return coup_pending_list;
}
    void Game::add_player(const std::shared_ptr<Player> &p)
    {
        assert_game_active();

        for (const auto &existing : players_list)
        {
            if (existing->get_name() == p->get_name())
            {
                throw DuplicatePlayerNameException(p->get_name());
            }
        }

        players_list.push_back(p);
        std::cout << "[Game] Added player: " << p->get_name() << " (" << p->role() << ")\n";
    }

    string Game::turn() const
    {
        if (players_list.empty())
        {
            throw InvalidActionException("No players in game.");
        }
        return players_list.at(current_turn_index)->get_name();
    }
    void Game::perform_action(const std::string &action_name, const std::string &by)
    {
        perform_action(action_name, by, "");
    }
    void Game::perform_action(const std::string &action_name, const std::string &by, const std::string &target_name)
    {
        assert_game_active();

        // ❌ אל תבדוק turn()
        last_actions[by] = action_name;

        auto actor = get_player_by_name(by);
        std::string log_line = "[" + action_name + "] performed by " + by +
                               " (Coins: " + std::to_string(actor->coins()) + ")";

        if (!target_name.empty())
        {
            try
            {
                auto target = get_player_by_name(target_name);
                log_line += " on " + target_name +
                            " (Coins: " + std::to_string(target->coins()) + ")";
            }
            catch (...)
            {
                log_line += " → " + target_name + " (Unknown)";
            }
        }

        std::cout << log_line << std::endl;
        this->log_action(log_line);
    }

    bool Game::can_undo_action(const std::string &target_name, const std::string &expected_action) const
    {
        auto it = last_actions.find(target_name);
        if (it == last_actions.end())
            return false;
        return it->second == expected_action;
    }

    void Game::cancel_last_action(const std::string &player_name)
    {
        last_actions.erase(player_name);

        std::string role;
        try
        {
            auto player = get_player_by_name(player_name);
            role = player->role();
        }
        catch (...)
        {
            role = "Unknown";
        }

        std::string action_name = "last action";
        if (role == "Judge")
            action_name = "bribe";
        else if (role == "Governor")
            action_name = "tax";

        std::string log_line = "[Undo] Cancelled " + action_name + " of: " + player_name;
        std::cout << log_line << std::endl;
        this->log_action(log_line);
    }

    void Game::end_game()
    {
        game_over = true;
        std::cout << "[Game] Game has ended.\n";
        this->log_action("[Game] Game has ended.");
    }

    vector<string> Game::players() const
    {
        vector<string> active;
        for (const auto &p : players_list)
        {
            if (p->is_active())
            {
                active.push_back(p->get_name());
            }
        }
        return active;
    }

    string Game::winner() const
    {
        vector<string> alive = players();
        if (alive.size() != 1)
        {
            throw GameNotOverException();
        }

        return alive.front();
    }

    void Game::next_turn()
    {
        assert_game_active();

        std::string prev = turn();
        // 🔍 בדוק אם נותר שחקן יחיד
        std::vector<std::string> alive = players();
        if (alive.size() == 1)
        {
            std::cout << "[Game] Winner is: " << alive.front() << std::endl;
            log_action("[Game] Winner is: " + alive.front());
            game_over = true;
            return;
        }

        size_t n = players_list.size();
        do
        {
            current_turn_index = (current_turn_index + 1) % n;
        } while (!players_list[current_turn_index]->is_active());

        coup_pending_list.erase(
            std::remove_if(
                coup_pending_list.begin(),
                coup_pending_list.end(),
                [&](const auto &entry)
                {
                    return entry.first == turn(); // השחקן הנוכחי בתור
                }),
            coup_pending_list.end());

        arrest_blocked_players.erase(prev);

        std::cout << "[Turn] " << prev << " ended. " << turn() << " begins.\n";

        if (current_turn_index == players_list.size() - 1)
        {
            undo_tax = false;
            undo_bribe = false;
            peek_disable = false;
            undo_tax = false;
            undo_coup = false;
            for (auto &p : players_list)
            {
                p->unsanction();
            }
        }
    }

    void Game::remove_player(const string &attacker, const string &victim)
    {
        assert_game_active();

        for (auto &p : players_list)
        {
            if (p->get_name() == victim)
            {
                
                
                p->set_active(false);

                std::cout << "[Eliminate] Player " << victim << " has been eliminated(unless undone by a general).\n";
                this->log_action("[Eliminate] Player " + victim + " has been eliminated(unless undone by a general).\n");

                return;
            }
        }
        throw PlayerNotFoundException(victim);
    }

    shared_ptr<Player> Game::get_player_by_name(const string &name)
    {
        assert_game_active();

        for (auto &p : players_list)
        {
            if (p->get_name() == name)
            {
                return p;
            }
        }
        throw PlayerNotFoundException(name);
    }

    void Game::block_arrest_for(const string &name)
    {
        assert_game_active();
        arrest_blocked_players.insert(name);
        perform_action("block_arrest", turn(), name);
    }

    bool Game::is_arrest_blocked(const string &name) const
    {
        return arrest_blocked_players.find(name) != arrest_blocked_players.end();
    }

    void Game::set_last_arrest_target(const string &attacker, const string &target)
    {
        assert_game_active();
        last_arrested = target;
        std::cout << "[Arrest] " << attacker << " arrested " << target << std::endl;
        perform_action("arrest", attacker, target);
    }

    bool Game::arrested_same_target(const string &target) const
    {
        return last_arrested == target;
    }



    void Game::print_state() const
    {
        using std::cout;
        using std::endl;

        cout << "\n===== Game State =====" << endl;
        cout << "Current turn: " << turn() << endl;

        cout << "\nActive players:" << endl;
        for (const auto &p : players_list)
        {
            if (p->is_active())
            {
                cout << " - " << p->get_name() << " (" << p->role() << ")"
                     << " | Coins: " << p->coins() << endl;
            }
        }

        cout << "\nEliminated players:" << endl;
        for (const auto &p : players_list)
        {
            if (!p->is_active())
            {
                cout << " - " << p->get_name() << " (" << p->role() << ")"
                     << " | Coins: " << p->coins() << endl;
            }
        }

        cout << "======================\n"
             << endl;
    }
    void Game:: cancel_coup(std:: string target)
    {
        assert_game_active();

        auto it = std::remove_if(coup_pending_list.begin(), coup_pending_list.end(),
                                 [&](const auto &entry) { return entry.second == target; });

        if (it != coup_pending_list.end())
        {
            coup_pending_list.erase(it, coup_pending_list.end());
            auto player_ptr = get_player_by_name(target);
            player_ptr->set_active(true); // החזרת השחקן לחיים
            std::cout << "[Coup] Coup on " << target << " has been cancelled.\n";
            this->log_action("[Coup] Coup on " + target + " has been cancelled.\n");
        }
        else
        {
            throw InvalidActionException("No pending coup on " + target);
        }
    }
    bool Game::is_coup_pending_on(const std::string& target) const{
        assert_game_active();
        return std::any_of(coup_pending_list.begin(), coup_pending_list.end(),
                           [&](const auto &entry) { return entry.second == target; });
    }
    void Game::reset()
    {
        players_list.clear();
        current_turn_index = 0;
        game_over = false;
        coup_pending_list.clear();
        arrest_blocked_players.clear();
        last_arrested = "";
        last_actions.clear();
        std::cout << "[Game] Reset complete.\n";
        this->log_action("[Game] Reset complete.\n");
    }

    bool Game::is_game_over() const
    {
        return game_over;
    }

} // namespace coup