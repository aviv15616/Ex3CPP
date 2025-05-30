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

/**
 * @brief Constructs a new Player and binds it to a game instance.
 * 
 * @param game_ref Reference to the game.
 * @param name The name of the player.
 */
Player::Player(Game &game_ref, const string &name)
    : name(name), game(&game_ref)
{
    std::cout << "[Init] Player created: " << name << std::endl;
}

// ============================
// 🔹 Accessors & State
// ============================

/**
 * @brief Returns the name of the player.
 */
const string &Player::get_name() const { return name; }

/**
 * @brief Returns the current coin count of the player.
 */
int Player::coins() const { return coin_count; }

/**
 * @brief Sets the player's coin count.
 * @throws InvalidActionException if amount is negative.
 */
void Player::set_coins(int amount)
{
    if (amount < 0)
    {
        throw InvalidActionException("Coin count cannot be negative.");
    }
    coin_count = amount;
}

/**
 * @brief Checks if the player is currently active.
 */
bool Player::is_active() const { return active; }

/**
 * @brief Marks the player as active or inactive.
 */
void Player::set_active(bool status) { active = status; }

/**
 * @brief Returns whether the player is under sanction.
 */
bool Player::is_sanctioned() const { return under_sanction; }

/**
 * @brief Returns whether the player is blocked from using arrest.
 */
bool Player::is_arrest_disabled() const { return arrest_disabled; }

/**
 * @brief Enables the ability to arrest for the player.
 */
void Player::enable_arrest() { arrest_disabled = false; }

/**
 * @brief Disables the ability to arrest for the player.
 */
void Player::disable_arrest() { arrest_disabled = true; }

// ============================
// 🔹 Primary Actions
// ============================

/**
 * @brief Player gathers 1 coin.
 * @throws NotYourTurnException if not in turn.
 * @throws InvalidActionException if under sanction or coup is required.
 */
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

/**
 * @brief Player skips their turn (only if coup not required).
 */
void Player::skip_turn()
{
    ensure_coup_required();
    game->perform_action("Skip Turn", name);
    game->next_turn();
}

/**
 * @brief Player gains 2 coins via tax.
 * @throws NotYourTurnException if not in turn.
 * @throws InvalidActionException if under sanction or coup is required.
 */
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

/**
 * @brief Player performs a bribe (costs 4 coins).
 * @throws NotYourTurnException or NotEnoughCoinsException.
 */
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

/**
 * @brief Player attempts to arrest another player.
 * @param target The player to arrest.
 * @throws various exceptions for invalid target or conditions.
 */
void Player::arrest(Player &target)
{
    if (game->turn() != name)
        throw NotYourTurnException();
    ensure_coup_required();

    if (target.get_name() == name)
        throw CannotTargetYourselfException("arrest");
    if (!target.is_active())
        throw PlayerAlreadyDeadException(target.get_name());
    if (target.coins() == 0 || (target.role() == "Merchant" && target.coins() < 2))
        throw InvalidActionException("Target doesn't have enough coins (" + std::to_string(target.coins()) + ").");
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

/**
 * @brief Player sanctions a target (costs 3 or 4 coins depending on target).
 * @param target The player to sanction.
 * @throws NotYourTurnException or NotEnoughCoinsException.
 */
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

/**
 * @brief Performs a coup against a target player.
 * @param target The player to eliminate.
 * @throws NotYourTurnException or NotEnoughCoinsException.
 */
void Player::coup(const Player &target)
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
// 🔹 Internal Utility
// ============================

/**
 * @brief Ensures a player is not bypassing a required coup.
 * @throws MustCoupWith10CoinsException if coins >= 10.
 */
void Player::ensure_coup_required() const
{
    if (coins() >= 10)
    {
        throw MustCoupWith10CoinsException();
    }
}

// ============================
// 🔹 Callbacks (for roles to override)
// ============================

/**
 * @brief Callback when player is arrested. Overridable.
 */
void Player::on_arrest() {}

/**
 * @brief Removes sanction status from player.
 */
void Player::unsanction() { under_sanction = false; }

/**
 * @brief Applies sanction status to player.
 */
void Player::on_sanction() { under_sanction = true; }

/**
 * @brief Called at the start of a player's turn.
 */
void Player::on_turn_start() {}

} // namespace coup
