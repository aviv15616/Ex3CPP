// test_player.cpp
// Anksilae@gmail.com

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Game.hpp"
#include "Player.hpp"
#include "Spy.hpp"
#include "Exceptions.hpp"

using namespace coup;

TEST_CASE("Basic player actions") {

    SUBCASE("Initial state") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        CHECK(p->get_name() == "Alice");
        CHECK(p->coins() == 0);
        CHECK(p->is_active());
        CHECK(p->role() == "Spy");
        CHECK_FALSE(p->is_sanctioned());
        CHECK_FALSE(p->is_arrest_disabled());
    }

    SUBCASE("gather adds 1 coin") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        g.set_current_turn_index(0);
        p->gather();
        CHECK(p->coins() == 1);
    }

    SUBCASE("gather throws if not your turn") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");
        g.next_turn();
        CHECK_THROWS_AS(p->gather(), NotYourTurnException);
    }

    SUBCASE("tax adds 2 coins") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        p->tax();
        CHECK(p->coins() == 2);
    }

    SUBCASE("tax/gather throws if under sanction") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");

        if (g.turn() != "Alice") {
            p2->skip_turn();
        }

        p->set_coins(3);
        p->on_sanction();

        CHECK_THROWS_AS(p->tax(), InvalidActionException);
        CHECK_THROWS_AS(p->gather(), InvalidActionException);

        g.next_turn();
        CHECK(g.turn() == "Bob");
    }

    SUBCASE("bribe removes 4 coins") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        p->set_coins(5);
        p->bribe();
        CHECK(p->coins() == 1);
    }

    SUBCASE("bribe throws if not enough coins") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        CHECK_THROWS_AS(p->bribe(), NotEnoughCoinsException);
    }

    SUBCASE("skip_turn updates turn index") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");
        size_t idx = g.get_current_turn_index();
        p->skip_turn();
        CHECK(g.get_current_turn_index() != idx);
    }

    SUBCASE("coup removes target and costs 7 coins") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");
        p->set_coins(7);
        p->coup(*p2);
        CHECK_FALSE(p2->is_active());
        CHECK(p->coins() == 0);
    }

    SUBCASE("coup throws if not enough coins") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");
        p->set_coins(6);
        CHECK_THROWS_AS(p->coup(*p2), NotEnoughCoinsException);
    }

    SUBCASE("sanction deducts coins and applies sanction") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");
        p->set_coins(5);
        p->sanction(*p2);
        CHECK(p->coins() <= 2);
        CHECK(p2->is_sanctioned());
    }

    SUBCASE("sanction throws if not enough coins") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");
        p->set_coins(2);
        CHECK_THROWS_AS(p->sanction(*p2), NotEnoughCoinsException);
    }

    SUBCASE("arrest steals coin") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");
        p2->set_coins(3);
        p->arrest(*p2);
        CHECK(p->coins() == 1);
        CHECK(p2->coins() == 2);
    }

    SUBCASE("arrest throws on same target again") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");
        g.set_current_turn_index(0);
        p2->set_coins(2);
        p->arrest(*p2);
        g.set_current_turn_index(0);
        p->set_coins(2);
        CHECK_THROWS_AS(p->arrest(*p2), InvalidActionException);
    }

    SUBCASE("arrest throws on self-target") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        p->set_coins(2);
        CHECK_THROWS_AS(p->arrest(*p), CannotTargetYourselfException);
    }

    SUBCASE("arrest throws if target is already dead") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");
        p2->set_active(false);
        CHECK_THROWS_AS(p->arrest(*p2), PlayerAlreadyDeadException);
    }

    SUBCASE("arrest throws if target has no coins") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");
        p2->set_coins(0);
        CHECK_THROWS_AS(p->arrest(*p2), InvalidActionException);
    }

    SUBCASE("enable/disable arrest works") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        p->disable_arrest();
        CHECK(p->is_arrest_disabled());
        p->enable_arrest();
        CHECK_FALSE(p->is_arrest_disabled());
    }

    SUBCASE("ensure_coup_required throws when coins >= 10") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        p->set_coins(10);
        CHECK_THROWS_AS(p->ensure_coup_required(), MustCoupWith10CoinsException);
    }

    SUBCASE("set_coins throws on negative value") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        CHECK_THROWS_AS(p->set_coins(-1), InvalidActionException);
    }

    SUBCASE("unsanction clears sanction") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        p->on_sanction();
        p->unsanction();
        CHECK_FALSE(p->is_sanctioned());
    }

    SUBCASE("primary actions throw when not your turn") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");
        g.set_current_turn_index(1);
        p->set_coins(10);
        p2->set_coins(3);
        CHECK_THROWS_AS(p->tax(), NotYourTurnException);
        CHECK_THROWS_AS(p->bribe(), NotYourTurnException);
        CHECK_THROWS_AS(p->gather(), NotYourTurnException);
        CHECK_THROWS_AS(p->coup(*p2), NotYourTurnException);
        CHECK_THROWS_AS(p->sanction(*p2), NotYourTurnException);
        CHECK_THROWS_AS(p->arrest(*p2), NotYourTurnException);
    }

    SUBCASE("arrest throws when blocked or on same target twice") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");

        g.set_current_turn_index(0);
        p2->set_coins(3);
        g.block_arrest_for("Alice");
        CHECK_THROWS_AS(p->arrest(*p2), InvalidActionException);

        g.next_turn(); // Bob
        g.next_turn(); // Alice
        p->arrest(*p2);

        g.set_current_turn_index(0);
        p->set_coins(3);
        CHECK_THROWS_AS(p->arrest(*p2), InvalidActionException);
    }

    SUBCASE("sanction on Judge costs 5 if not enough coins") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto judge = g.add_player("Charlie", "Judge");

        p->set_coins(5);
        g.set_current_turn_index(0);
        p->sanction(*judge);
        CHECK(p->coins() == 1);

        g.set_current_turn_index(0);
        p->set_coins(3);
        CHECK_THROWS_AS(p->sanction(*judge), NotEnoughCoinsException);
    }

    SUBCASE("ensure_coup_required triggers correctly in all actions") {
        Game g;
        auto p = g.add_player("Alice", "Spy");
        auto p2 = g.add_player("Bob", "Spy");

        p->set_coins(10);
        g.set_current_turn_index(0);

        CHECK_THROWS_AS(p->tax(), MustCoupWith10CoinsException);
        CHECK_THROWS_AS(p->bribe(), MustCoupWith10CoinsException);
        CHECK_THROWS_AS(p->gather(), MustCoupWith10CoinsException);
        CHECK_THROWS_AS(p->sanction(*p2), MustCoupWith10CoinsException);
        CHECK_THROWS_AS(p->arrest(*p2), MustCoupWith10CoinsException);
    }
}
