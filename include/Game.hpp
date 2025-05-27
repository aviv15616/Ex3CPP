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
        std::vector<std::shared_ptr<Player>> players_list;
        size_t current_turn_index = 0;
        bool game_over = false;

        std::string last_arrested;
std::vector<std::pair<std::string, std::string>> coup_pending_list;
        std::unordered_set<std::string> arrest_blocked_players;
       
        
        void assert_game_active() const;
        std::string last_action;



    public:
        Game();

        // ניהול שחקנים
        std::shared_ptr<Player> add_player(const std::string& name, const std::string& role);
        void add_to_coup(const std::string &attacker, const std::string &target);
        const std::vector<std::pair<std::string, std::string>> &get_coup_pending_list() const;
        void add_player(const std::shared_ptr<Player> &p); // מיועד לטסטים

        void remove_player(const std::string& attacker,const std::string& victim);
        std::shared_ptr<Player> get_player_by_name(const std::string& name);
        bool is_name_taken(const std::string& name) const;
        void log_action(const std::string& text);
        bool undo_tax=false;
        bool undo_bribe=false;
        bool peek_disable=false;
 
        bool undo_coup=false;

        std::string get_last_action() const { return last_action; }


        // תור
        std::string turn() const;
        void perform_action(const std::string &action_name, const std::string &by, const std::string &target_name);
        void next_turn();
        std::unordered_map<std::string, std::string> last_actions;

        // פעולות כלליות
        void perform_action(const std::string& action_name, const std::string& by);
        bool can_undo_action(const std::string& target_name, const std::string& expected_action) const;
        void cancel_last_action(const std::string& player_name);

        // סטטוס משחק
        std::vector<std::string> players() const;
        std::string winner() const;

        // פונקציות coup
        void mark_coup_pending(const std::string& attacker, const std::string& target);
        bool is_coup_pending_on(const std::string& target) const;
        std::string get_coup_attacker(const std::string& target) const;
        void cancel_coup(std::string target);

        void print_state() const;

        void reset();
        void end_game();

        bool is_game_over() const;

        // arrest control
        void block_arrest_for(const std::string& name);
        bool is_arrest_blocked(const std::string& name) const;
        void set_last_arrest_target(const std::string& attacker, const std::string& target);
        bool arrested_same_target(const std::string& target) const;
    };

}
