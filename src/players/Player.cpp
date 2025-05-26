#include "Player.hpp"
#include "../core/Game.hpp"
#include "../core/Exceptions.hpp"
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
        if (game->turn() != name) {
            throw NotYourTurnException();
        }
        ensure_coup_required();
        on_turn_start();
        game->perform_action("gather", name);
        coin_count += 1;
        std::cout << "[Gather] " << name << " gains 1 coin. Total: " << coin_count << std::endl;
        game->next_turn();
    }

    void Player::tax() {
        if (game->turn() != name) {
            throw NotYourTurnException();
        }
        ensure_coup_required();
        on_turn_start();
        game->perform_action("tax", name);
        coin_count += 2;
        std::cout << "[Tax] " << name << " gains 2 coins. Total: " << coin_count << std::endl;
        game->next_turn();
    }

    void Player::bribe() {
        if (game->turn() != name) {
            throw NotYourTurnException();
        }
        ensure_coup_required();
        const int cost = 4;
        if (coin_count < cost) {
            throw NotEnoughCoinsException(cost, coin_count);
        }
        game->perform_action("bribe", name);
        coin_count -= cost;
        std::cout << "[Bribe] " << name << " pays 4 coins. Remaining: " << coin_count << std::endl;
    }

    void Player::arrest(Player& target) {
        if (game->turn() != name) {
            throw NotYourTurnException();
        }
        ensure_coup_required();

        if (target.get_name() == name) {
            throw CannotTargetYourselfException("arrest");
        }
        if (!target.is_active()) {
            throw PlayerAlreadyDeadException(target.get_name());
        }
        if (target.coins() == 0) {
            throw InvalidActionException("Target has no coins to steal.");
        }
        if (game->is_arrest_blocked(this->get_name())) {
            throw InvalidActionException("You are blocked from using arrest this turn.");
        }
        if (game->arrested_same_target_last_turn(this->get_name(), target.get_name())) {
            throw InvalidActionException("Cannot arrest the same player twice in a row.");
        }

        game->perform_action("arrest", name);
        target.on_arrest();
        target.set_coins(target.coins() - 1);
        set_coins(coins() + 1);
        game->set_last_arrest_target(this->get_name(), target.get_name());
        game->next_turn();
    }

    void Player::sanction(Player& target) {
        if (game->turn() != name) {
            throw NotYourTurnException();
        }
        ensure_coup_required();

        const int cost = 3;
        if (coin_count < cost) {
            throw NotEnoughCoinsException(cost, coin_count);
        }

        game->perform_action("sanction", name);
        target.on_sanction();

        if (target.role() == "Judge") {
            if (coin_count < 1) {
                throw NotEnoughCoinsException(1, coin_count);
            }
            coin_count -= 1;
            std::cout << "[Sanction] " << name << " pays extra 1 coin for sanctioning Judge." << std::endl;
        }

        coin_count -= cost;
        std::cout << "[Sanction] " << name << " pays 3 coins. Remaining: " << coin_count << std::endl;
        game->next_turn();
    }

    void Player::coup(Player& target) {
        if (game->turn() != name) {
            throw NotYourTurnException();
        }
        const int cost = 7;
        if (coin_count < cost) {
            throw NotEnoughCoinsException(cost, coin_count);
        }
        game->perform_action("coup", name);
        std::cout << "[Coup] " << name << " eliminates " << target.get_name() << std::endl;
        target.set_active(false);
        coin_count -= cost;
        game->next_turn();
    }

    void Player::undo_tax(Player& target) {
        if (!game->can_undo_action(target.get_name(), "tax")) {
            throw UndoNotAllowed(role(), "undo_tax");
        }
        target.set_coins(target.coins() - 2);
        this->set_coins(this->coins() + 2);
        std::cout << "[Undo] " << name << " undoes tax by " << target.get_name() << std::endl;
        game->cancel_last_action(target.get_name());
    }

    void Player::undo_bribe(Player& target) {
        if (!game->can_undo_action(target.get_name(), "bribe")) {
            throw UndoNotAllowed(role(), "undo_bribe");
        }
        target.set_coins(target.coins() + 4);
        this->set_coins(this->coins() - 4);
        std::cout << "[Undo] " << name << " undoes bribe by " << target.get_name() << std::endl;
        game->cancel_last_action(target.get_name());
    }

    void Player::ensure_coup_required() {
        if (coins() >= 10) {
            throw InvalidActionException("You must perform a coup when holding 10 or more coins.");
        }
    }

    void Player::on_arrest() {
    }

    void Player::on_sanction() {
    }

    void Player::on_turn_start() {
    }

} // namespace coup
