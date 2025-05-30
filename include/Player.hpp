// Anksilae@gmail.com

#pragma once

#include <string>
#include <memory>

namespace coup {

class Game; // Forward declaration to avoid including Game.hpp

/**
 * @brief Abstract base class representing a player in the Coup game.
 * 
 * Contains core logic shared by all roles: coin management, actions (gather, bribe, coup),
 * and hooks for passive/active role-specific behavior.
 */
class Player {
protected:
    std::string name;        ///< Player name
    Game* game;              ///< Pointer to the game instance
    int coin_count = 0;      ///< Number of coins the player currently holds
    bool active = true;      ///< Whether the player is alive in the game
    bool under_sanction = false;   ///< Whether player is currently sanctioned
    bool arrest_disabled = false; ///< Whether arrest is blocked for this player

public:
    /**
     * @brief Constructs a new Player with the given name and game reference.
     * @param game_ref Reference to the game.
     * @param name Name of the player.
     */
    Player(Game& game_ref, const std::string& name);

    /// Virtual destructor
    virtual ~Player() = default;

    // ===== State and Info Accessors =====

    /**
     * @brief Returns the player's name.
     */
    const std::string& get_name() const;

    /**
     * @brief Returns the number of coins the player has.
     */
    int coins() const;

    /**
     * @brief Sets the player's coin count.
     * @throws InvalidActionException if amount is negative.
     */
    void set_coins(int amount);

    /**
     * @brief Returns true if the player is active (alive).
     */
    bool is_active() const;

    /**
     * @brief Sets the player's active status (alive or eliminated).
     */
    void set_active(bool status);

    // ===== Abstract Methods =====

    /**
     * @brief Returns the role name (must be implemented by derived class).
     */
    virtual std::string role() const = 0;

    // ===== Basic Actions =====

    /**
     * @brief Gain 1 coin (does not target another player).
     * @throws NotYourTurnException or InvalidActionException if under sanction or must coup.
     */
    void gather();

    /**
     * @brief Skip the turn voluntarily.
     * @throws MustCoupWith10CoinsException if player has ≥10 coins.
     */
    void skip_turn();

    /**
     * @brief Gain 2 coins (can be overridden by role).
     * @throws NotYourTurnException or InvalidActionException if under sanction or must coup.
     */
    virtual void tax();

    /**
     * @brief Spend 4 coins to bribe (used by some roles).
     * @throws NotYourTurnException or NotEnoughCoinsException.
     */
    void bribe();

    // ===== Targeted Actions =====

    /**
     * @brief Arrest another player and potentially steal coins.
     * @throws Multiple exceptions depending on conditions (self-targeting, blocked, etc).
     */
    void arrest(Player& target);

    /**
     * @brief Apply sanction on another player.
     * @throws NotYourTurnException or NotEnoughCoinsException.
     */
    void sanction(Player& target);

    /**
     * @brief Eliminate another player (coup).
     * @throws NotYourTurnException or NotEnoughCoinsException.
     */
    void coup(const Player& target);

    /**
     * @brief Returns true if the player is currently sanctioned.
     */
    bool is_sanctioned() const;

    /**
     * @brief Returns true if the player's ability to arrest is blocked.
     */
    bool is_arrest_disabled() const;

    /**
     * @brief Enable arrest ability (typically called at start of turn).
     */
    void enable_arrest();

    /**
     * @brief Disable arrest ability (used by Spy or sanction effects).
     */
    void disable_arrest();

    /**
     * @brief Throws if player has 10+ coins and has not performed a coup.
     * @throws MustCoupWith10CoinsException.
     */
    void ensure_coup_required() const;

    // ===== Turn Start Hook =====

    /**
     * @brief Hook called at the start of the player's turn.
     * Default: no effect (roles like Merchant override it).
     */
    virtual void on_turn_start();

    // ===== Passive Reactions =====

    /**
     * @brief Hook called when the player is arrested.
     * Default: no effect (roles like General override it).
     */
    virtual void on_arrest();

    /**
     * @brief Remove sanction from the player.
     */
    void unsanction();

    /**
     * @brief Hook called when player receives a sanction.
     * Default: sets `under_sanction = true`.
     */
    virtual void on_sanction();
};

} // namespace coup
