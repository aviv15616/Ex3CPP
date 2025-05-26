#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include "../players/Player.hpp"

namespace coup {

    class Game {
    private:
        std::vector<std::shared_ptr<Player>> players_list;
        size_t current_turn_index = 0;
        bool game_over = false;

        std::optional<std::pair<std::string, std::string>> last_coup_pending;
        std::unordered_set<std::string> arrest_blocked_players;
        std::unordered_map<std::string, std::string> last_arrest_target;
        std::unordered_map<std::string, std::string> last_actions;

    public:
        Game();

        // ניהול שחקנים
        void add_player(const std::shared_ptr<Player>& player);
        void remove_player(const std::string& name);
        std::shared_ptr<Player> get_player_by_name(const std::string& name);

        // תור
        std::string turn() const;
        void next_turn();

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
        void cancel_coup();

        void print_state() const;

        void reset();

        bool is_game_over() const;

        // arrest control
        void block_arrest_for(const std::string& name);
        bool is_arrest_blocked(const std::string& name) const;
        void set_last_arrest_target(const std::string& attacker, const std::string& target);
        bool arrested_same_target_last_turn(const std::string& attacker, const std::string& target) const;
    };

}
