#include "Player.hpp"
#include "Game.hpp"
#include "Exceptions.hpp"
#include <iostream>

using namespace std;

namespace coup {

    Player::Player(Game& game_ref, const string& name)
        : name(name), game(&game_ref) {
        std::cout << "[Init] Player created: " << name << std::endl;
    }

    const string& Player::get_name() const {
        return name;
    }

    int Player::coins() const {
        return coin_count;
    }

    void Player::set_coins(int amount) {
        if (amount < 0) {
            throw InvalidActionException("Coin count cannot be negative.");
        }
        coin_count = amount;
        std::cout << "[Coins] " << name << " now has " << coin_count << " coins." << std::endl;
    }

    bool Player::is_active() const {
        return active;
    }

    void Player::set_active(bool status) {
        active = status;
    }

    void Player::gather() {
        if (game->turn() != name) throw NotYourTurnException();
        if (under_sanction) throw InvalidActionException("You are under sanction and cannot use Gather/Tax this turn.");
        ensure_coup_required();
        set_coins(coins() + 1);
        game->perform_action("gather", name);
        std::cout << "[Gather] " << name << " gains 1 coin. Total: " << coin_count << std::endl;
        game->next_turn();
    }
    
    void Player::tax() {
        if (game->turn() != name) throw NotYourTurnException();
        if (under_sanction) throw InvalidActionException("You are under sanction and cannot use Gather/Tax this turn.");
        ensure_coup_required();
        set_coins(coins() + 2);
        game->perform_action("tax", name);
        std::cout << "[Tax] " << name << " gains 2 coins. Total: " << coin_count << std::endl;
        game->next_turn();
    }

    void Player::bribe() {
        if (game->turn() != name) throw NotYourTurnException();
        ensure_coup_required();
        const int cost = 4;
        if (coins() < cost) throw NotEnoughCoinsException(cost, coins());

        set_coins(coins() - cost);
        game->perform_action("bribe", name);
        std::cout << "[Bribe] " << name << " pays 4 coins, and gains extra turn. Remaining: " << coin_count << std::endl;
    }

    void Player::arrest(Player& target) {
        if (game->turn() != name) throw NotYourTurnException();
        ensure_coup_required();

        if (target.get_name() == name) throw CannotTargetYourselfException("arrest");
        if (!target.is_active()) throw PlayerAlreadyDeadException(target.get_name());
        if (target.coins() == 0) throw InvalidActionException("Target has no coins to steal.");
        if (game->is_arrest_blocked(name)) throw InvalidActionException("You are blocked from using arrest this turn.");
        if (game->arrested_same_target(target.get_name())) throw InvalidActionException("Cannot arrest the same player twice in a row.");

        game->perform_action("arrest", name, target.get_name());
        target.on_arrest();
        target.set_coins(target.coins() - 1);
        set_coins(coins() + 1);
        game->set_last_arrest_target(name, target.get_name());
        game->next_turn();
    }

    void Player::sanction(Player& target) {
        if (game->turn() != name) throw NotYourTurnException();
        ensure_coup_required();

        const int cost = 3;
        if (coins() < cost) throw NotEnoughCoinsException(cost, coins());

        game->perform_action("sanction", name, target.get_name());
        target.on_sanction();

        int total_cost = cost;

        if (target.role() == "Judge") {
            if (coins() < cost + 1) {
                throw NotEnoughCoinsException(cost + 1, coins());
            }
            total_cost += 1;
            std::cout << "[Sanction] " << name << " pays extra 1 coin for sanctioning Judge." << std::endl;
        }

        set_coins(coins() - total_cost);
        std::cout << "[Sanction] " << name << " pays " << total_cost << " coins. Remaining: " << coin_count << std::endl;
        game->next_turn();
    }

  void Player::coup(Player& target) {
    if (game->turn() != name) throw NotYourTurnException();
    const int cost = 7;
    if (coins() < cost) throw NotEnoughCoinsException(cost, coins());

    std::cout << "[Coup] " << name << " eliminates " << target.get_name() << std::endl;

    game->remove_player(this->get_name(), target.get_name());

    set_coins(coins() - cost);
    game->perform_action("coup", name, target.get_name());

    game->add_to_coup(this->get_name(), target.get_name()); // ✅ הוספה לרשימת pending

    game->next_turn();
}

    void Player::undo_tax(Player&) {}
    void Player::undo_bribe(Player&) {}

    void Player::ensure_coup_required() {
        if (coins() >= 10) {
            throw InvalidActionException("You must perform a coup when holding 10 or more coins.");
        }
    }

    void Player::on_arrest() {
     
    }
    void Player:: unsanction(){
            under_sanction = false;

    }

    void Player::on_sanction() {
        under_sanction = true;
    }

    void Player::on_turn_start() {
       
    }

} // namespace coup
