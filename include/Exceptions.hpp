#pragma once

#include <stdexcept>
#include <string>

namespace coup {

    // בסיס לכל חריגה מותאמת אישית
    class CoupException : public std::runtime_error {
    public:
        explicit CoupException(const std::string& msg)
            : std::runtime_error(msg) {}
    };

    // 🔹 חריגות כלליות
    class NotYourTurnException : public CoupException {
    public:
        NotYourTurnException() : CoupException("It's not your turn.") {}
    };

    class InvalidActionException : public CoupException {
    public:
        explicit InvalidActionException(const std::string& reason)
            : CoupException("Invalid action: " + reason) {}
    };

    class GameNotOverException : public CoupException {
    public:
        GameNotOverException() : CoupException("Game is not over yet.") {}
    };

    class GameAlreadyOverException : public CoupException {
    public:
        GameAlreadyOverException() : CoupException("Game is already over.") {}
    };

    // 🔹 חריגות שחקן
    class PlayerNotFoundException : public CoupException {
    public:
        explicit PlayerNotFoundException(const std::string& name)
            : CoupException("Player not found: " + name) {}
    };
    class DuplicatePlayerNameException : public CoupException {
    public:
        explicit DuplicatePlayerNameException(const std::string& name)
            : CoupException("Player name already exists: " + name) {}
    };

    class PlayerAlreadyDeadException : public CoupException {
    public:
        explicit PlayerAlreadyDeadException(const std::string& name)
            : CoupException("Player is already out: " + name) {}
    };

    class CannotTargetYourselfException : public CoupException {
    public:
        explicit CannotTargetYourselfException(const std::string& action)
            : CoupException("Cannot " + action + " yourself.") {}
    };

    // 🔹 חריגות כספיות
    class NotEnoughCoinsException : public CoupException {
    public:
        NotEnoughCoinsException(int required, int current)
            : CoupException("Not enough coins. Required: " + std::to_string(required) + ", but have: " + std::to_string(current)) {}
    };

    class MustCoupWith10CoinsException : public CoupException {
    public:
        MustCoupWith10CoinsException()
            : CoupException("Player has 10 or more coins and must perform a coup.") {}
    };

    // 🔹 חריגות undo / חסימות
    class UndoNotAllowed : public CoupException {
    public:
        UndoNotAllowed(const std::string& role, const std::string& action)
            : CoupException(role + " cannot perform " + action + ".") {}
    };

    class BlockNotAvailableException : public CoupException {
    public:
        BlockNotAvailableException(const std::string& action)
            : CoupException("No player is allowed to block action: " + action) {}
    };

    // 🔹 חריגות חוזרות (arrest על אותו שחקן, sanction חוזר)
    class ActionAlreadyUsedOnPlayerException : public CoupException {
    public:
        ActionAlreadyUsedOnPlayerException(const std::string& action, const std::string& target)
            : CoupException("Cannot use action '" + action + "' again on player '" + target + "' consecutively.") {}
    };

    // 🔹 חריגות פנימיות (שימוש בעתיד אפשרי)
    class InternalLogicException : public CoupException {
    public:
        InternalLogicException(const std::string& msg)
            : CoupException("Internal error: " + msg) {}
    };
}
