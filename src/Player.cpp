// Player.cpp
// מימוש פעולות בסיסיות עבור שחקן במשחק Coup
// Anksilae@gmail.com

#include "Player.hpp"
#include "Game.hpp"
#include "Exceptions.hpp"
#include <iostream>

using namespace std;

namespace coup
{

    // ============================
    // 🔹 Constructor
    // ============================

    Player::Player(Game &game_ref, const string &name)
        : name(name), game(&game_ref)
    {
        std::cout << "[Init] Player created: " << name << std::endl;
    }

    // ============================
    // 🔹 Accessors & State
    // ============================

    const string &Player::get_name() const
    {
        return name;
    }

    int Player::coins() const
    {
        return coin_count;
    }

    void Player::set_coins(int amount)
    {
        if (amount < 0)
        {
            throw InvalidActionException("Coin count cannot be negative.");
        }
        coin_count = amount;
    }

    bool Player::is_active() const
    {
        return active;
    }

    void Player::set_active(bool status)
    {
        active = status;
    }

    bool Player::is_sanctioned() const
    {
        return under_sanction;
    }
    bool Player::is_arrest_disabled() const
    {
        return arrest_disabled;
    }
    void Player::enable_arrest()
    {
        arrest_disabled = false;
    }
    void Player::disable_arrest() { 
        arrest_disabled = true; 
    } 

    
   
    // ============================
    // 🔹 Primary Actions
    // ============================

    void Player::gather()
    {
        if (game->turn() != name)
            throw NotYourTurnException();
        if (under_sanction)
            throw InvalidActionException("You are under sanction and cannot use Gather/Tax this turn.");
        ensure_coup_required();
        set_coins(coins() + 1);
        game->perform_action("gather", name);
        game->next_turn();
    }
    void Player::skip_turn(){
        ensure_coup_required();
        game->perform_action("Skip Turn", name);

        game->next_turn();
        
    }

    void Player::tax()
    {
        if (game->turn() != name)
            throw NotYourTurnException();
        if (under_sanction)
            throw InvalidActionException("You are under sanction and cannot use Gather/Tax this turn.");
        ensure_coup_required();
        set_coins(coins() + 2);
        game->perform_action("tax", name);
        game->next_turn();
    }

    void Player::bribe()
    {
        if (game->turn() != name)
            throw NotYourTurnException();
        ensure_coup_required();
        const int cost = 4;
        if (coins() < cost)
            throw NotEnoughCoinsException(cost, coins());
        set_coins(coins() - cost);
        game->perform_action("bribe", name);
    }

    // ============================
    // 🔹 Targeted Actions
    // ============================

    void Player::arrest(Player &target)
    {
        if (game->turn() != name)
            throw NotYourTurnException();
        ensure_coup_required();

        if (target.get_name() == name)
            throw CannotTargetYourselfException("arrest");
        if (!target.is_active())
            throw PlayerAlreadyDeadException(target.get_name());
        if (target.coins() == 0||(target.role() == "Merchant"&&target.coins() < 2))
            throw InvalidActionException("Target doesn't have enough coins (" + std::to_string(target.coins()) + ")." );
        if (game->is_arrest_blocked(name))
            throw InvalidActionException("You are blocked from using arrest this turn.");
        if (game->arrested_same_target(target.get_name()))
            throw InvalidActionException("Cannot arrest the same player twice in a row.");

        target.on_arrest();
        if (target.role() == "Merchant")
        {
            target.set_coins(target.coins() - 2);
        }
        else
        {
            target.set_coins(target.coins() - 1);
            set_coins(coins() + 1);
        }

        game->set_last_arrest_target(target.get_name());
        game->perform_action("arrest", name, target.get_name());
        game->next_turn();
    }

    void Player::sanction(Player &target)
    {
        if (game->turn() != name)
            throw NotYourTurnException();
        ensure_coup_required();

        const int cost = 3;
        if (coins() < cost)
            throw NotEnoughCoinsException(cost, coins());

        target.on_sanction();
        int total_cost = cost;

        if (target.role() == "Judge")
        {
            if (coins() < cost + 1)
                throw NotEnoughCoinsException(cost + 1, coins());
            total_cost += 1;
        }

        set_coins(coins() - total_cost);
        game->perform_action("sanction", name, target.get_name());
        game->next_turn();
    }

    void Player::coup(Player &target)
    {
        if (game->turn() != name)
            throw NotYourTurnException();
        const int cost = 7;
        if (coins() < cost)
            throw NotEnoughCoinsException(cost, coins());

        game->remove_player(target.get_name());
        set_coins(coins() - cost);
        game->perform_action("coup", name, target.get_name());
        game->add_to_coup(this->get_name(), target.get_name());
        game->next_turn();
    }

    // ============================
    // 🔹 Undo-able Stubs (overridden in roles)
    // ============================

    void Player::undo_tax(Player &) {}
    void Player::undo_bribe(Player &) {}

    // ============================
    // 🔹 Internal Utility
    // ============================

    void Player::ensure_coup_required()
    {
        if (coins() >= 10)
        {
            throw InvalidActionException("You must perform a coup when holding 10 or more coins.");
        }
    }

    // ============================
    // 🔹 Callbacks (for roles to override)
    // ============================

    void Player::on_arrest() {}
    void Player::unsanction() { under_sanction = false; }
    void Player::on_sanction() { under_sanction = true; }
    void Player::on_turn_start() {}

} // namespace coup
