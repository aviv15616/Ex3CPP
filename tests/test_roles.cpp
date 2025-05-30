// test_roles.cpp
// Anksilae@gmail.com

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Game.hpp"
#include "Governor.hpp"
#include "Spy.hpp"
#include "Baron.hpp"
#include "General.hpp"
#include "Judge.hpp"
#include "Merchant.hpp"
#include "Exceptions.hpp"

using namespace coup;

void force_turn(Game& g, const std::string& name) {
    for (size_t i = 0; i < g.players().size() + 5; ++i) {
        if (g.turn() == name) return;
        g.next_turn();
    }
}

TEST_CASE("Governor functionality and edge cases") {
    Game g;
    auto gov = std::make_shared<Governor>(g, "Gov");
    auto gov2 = std::make_shared<Governor>(g, "OtherGov");
    g.add_player(gov);
    g.add_player(gov2);

    force_turn(g, "Gov");
    gov->tax();
    CHECK(gov->coins() == 3);

    force_turn(g, "OtherGov");
 
    gov2->set_coins(3);
    g.perform_action("tax", gov2->get_name());

    CHECK_NOTHROW(gov->undo_tax(*gov2));
 
    CHECK(gov2->coins() == 0);

    g.next_turn();
    g.next_turn(); // reset undo flag
    gov2->set_coins(1);
    g.perform_action("tax", gov2->get_name());
    g.undo_tax = true; // simulate already undone
    CHECK_THROWS_AS(gov->undo_tax(*gov2), InvalidActionException);
}
TEST_CASE("Governor::undo_tax throws when targeting self") {
    Game g;
    auto gov = std::make_shared<Governor>(g, "Gov");
    g.add_player(gov);

    g.perform_action("tax", gov->get_name());  // פעולה חוקית כדי ש־can_undo_action יעבור
    force_turn(g, "Gov");

    CHECK_THROWS_AS(gov->undo_tax(*gov), CannotTargetYourselfException);
}
TEST_CASE("Governor::undo_tax throws if target has insufficient coins") {
    Game g;
    auto gov = std::make_shared<Governor>(g, "Gov");
    auto gov2 = std::make_shared<Governor>(g, "OtherGov");
    g.add_player(gov);
    g.add_player(gov2);

    gov2->set_coins(2);  // פחות מהעלות של Governor (3)
    force_turn(g, "OtherGov");
    g.perform_action("tax", gov2->get_name());

    force_turn(g, "Gov");
    CHECK_THROWS_AS(gov->undo_tax(*gov2), NotEnoughCoinsException);
}


TEST_CASE("Governor::tax throws if not your turn") {
    Game g;
    auto gov1 = std::make_shared<Governor>(g, "Gov1");
    auto gov2 = std::make_shared<Governor>(g, "Gov2");
    g.add_player(gov1);
    g.add_player(gov2);
    force_turn(g, "Gov2");
    CHECK_THROWS_AS(gov1->tax(), NotYourTurnException);
}

TEST_CASE("Governor::tax throws when under sanction") {
    Game g;
    auto gov = std::make_shared<Governor>(g, "Gov");
    g.add_player(gov);

    gov->on_sanction(); // החלת סנקציה
    force_turn(g, "Gov");

    CHECK_THROWS_AS(gov->tax(), InvalidActionException);
}

TEST_CASE("Governor::undo_tax throws if undo not allowed") {
    Game g;
    auto gov = std::make_shared<Governor>(g, "Gov");
    auto other = std::make_shared<Governor>(g, "OtherGov");
    g.add_player(gov);
    g.add_player(other);
    force_turn(g, "Gov");
    CHECK_THROWS_AS(gov->undo_tax(*other), UndoNotAllowed);
}

TEST_CASE("Baron invest and sanction behavior") {
    Game g;
    auto b = std::make_shared<Baron>(g, "Bar");
    g.add_player(b);

    force_turn(g, "Bar");
    b->set_coins(3);
    b->invest();
    CHECK(b->coins() == 6);

    b->on_sanction();
    CHECK(b->coins() == 7);
}

TEST_CASE("Baron::invest throws if not your turn") {
    Game g;
    auto baron = std::make_shared<Baron>(g, "Baron");
    auto other = std::make_shared<Governor>(g, "Other");
    g.add_player(baron);
    g.add_player(other);
    force_turn(g, "Other");
    baron->set_coins(3);
    CHECK_THROWS_AS(baron->invest(), NotYourTurnException);
}

TEST_CASE("Baron::invest throws if not enough coins") {
    Game g;
    auto baron = std::make_shared<Baron>(g, "Baron");
    g.add_player(baron);
    force_turn(g, "Baron");
    baron->set_coins(2);
    CHECK_THROWS_AS(baron->invest(), NotEnoughCoinsException);
}

TEST_CASE("General coup block and arrest compensation") {
    Game g;
    auto gen = std::make_shared<General>(g, "Gen");
    auto victim = std::make_shared<Spy>(g, "Victim");
    g.add_player(gen);
    g.add_player(victim);

    g.add_to_coup("Someone", victim->get_name());
    gen->set_coins(5);
    CHECK_NOTHROW(gen->undo_coup(*victim));
    CHECK_FALSE(g.is_coup_pending_on(victim->get_name()));

    gen->set_coins(0);
    gen->on_arrest();
    CHECK(gen->coins() == 1);
}
TEST_CASE("General::undo_coup throws if already undone this round") {
    Game g;
    auto gen = std::make_shared<General>(g, "Gen");
    auto victim = std::make_shared<Spy>(g, "Target");
    g.add_player(gen);
    g.add_player(victim);

    g.add_to_coup("Someone", victim->get_name());
    gen->set_coins(5);
    CHECK_NOTHROW(gen->undo_coup(*victim));

    g.add_to_coup("Someone", victim->get_name());
    g.undo_coup = true; // סימולציה של undo קודם
    gen->set_coins(5);
    CHECK_THROWS_AS(gen->undo_coup(*victim), InvalidActionException);
}


TEST_CASE("General::undo_coup throws if not enough coins") {
    Game g;
    auto general = std::make_shared<General>(g, "Gen");
    auto victim = std::make_shared<Spy>(g, "Victim");
    g.add_player(general);
    g.add_player(victim);
    g.add_to_coup("Someone", victim->get_name());
    general->set_coins(4);
    CHECK_THROWS_AS(general->undo_coup(*victim), NotEnoughCoinsException);
}

TEST_CASE("General::block_coup throws if no coup pending") {
    Game g;
    auto general = std::make_shared<General>(g, "Gen");
    auto victim = std::make_shared<Spy>(g, "Victim");
    g.add_player(general);
    g.add_player(victim);
    general->set_coins(5);
    CHECK_THROWS_AS(general->undo_coup(*victim), InvalidActionException);
}
TEST_CASE("Judge::undo_bribe throws when targeting self") {
    Game g;
    auto judge = std::make_shared<Judge>(g, "Judge");
    g.add_player(judge);

    // מכין פעולה שנראית חוקית כדי לעבור את תנאי can_undo_action
    g.perform_action("bribe", judge->get_name());
    force_turn(g, "Judge");

    CHECK_THROWS_AS(judge->undo_bribe(*judge), CannotTargetYourselfException);
}
TEST_CASE("Judge::undo_bribe throws if no bribe action was made") {
    Game g;
    auto judge = std::make_shared<Judge>(g, "Judge");
    auto target = std::make_shared<Spy>(g, "SpyGuy");
    g.add_player(judge);
    g.add_player(target);

    force_turn(g, "SpyGuy");
    target->gather(); // פעולה אחרת ולא bribe
    force_turn(g, "Judge");

    CHECK_THROWS_AS(judge->undo_bribe(*target), UndoNotAllowed);
}


TEST_CASE("Judge undo_bribe and sanction logic") {
    Game g;
    auto judge = std::make_shared<Judge>(g, "Judge");
    auto bribing_player = std::make_shared<Spy>(g, "Spyyy");
    g.add_player(judge);
    g.add_player(bribing_player);

    bribing_player->set_coins(4);
    force_turn(g, "Spyyy");
    bribing_player->bribe();
    g.perform_action("bribe", bribing_player->get_name());
    CHECK(bribing_player->coins() == 0);
    force_turn(g, "Judge");
   
    CHECK_NOTHROW(judge->undo_bribe(*bribing_player));
    CHECK(bribing_player->coins() == 0);

    bribing_player->set_coins(1);
    g.perform_action("bribe", bribing_player->get_name());
    CHECK_THROWS_AS(judge->undo_bribe(*bribing_player), InvalidActionException);
}

TEST_CASE("Merchant turn start bonus and arrest penalty") {
    Game g;
    auto m = std::make_shared<Merchant>(g, "Merch");
     auto b = std::make_shared<Spy>(g, "Merch2");
     b->set_coins(2);
    g.add_player(m);
    g.add_player(b);
    m->set_coins(3);
    m->on_turn_start();
    CHECK(m->coins() == 4);
    force_turn(g,"Merch2");
    b->arrest(*m);

    CHECK(m->coins() == 2);

}

TEST_CASE("Sanction on Judge costs extra") {
    Game g;
    auto attacker = std::make_shared<Governor>(g, "Gov");
    auto judge = std::make_shared<Judge>(g, "Judgey");
    g.add_player(attacker);
    g.add_player(judge);

    force_turn(g, "Gov");
    attacker->set_coins(5);
    CHECK_NOTHROW(attacker->sanction(*judge));
    CHECK(attacker->coins() == 1);
}

TEST_CASE("Sanction on Judge fails if player lacks extra coin") {
    Game g;
    auto attacker = std::make_shared<Governor>(g, "Gov");
    auto judge = std::make_shared<Judge>(g, "Judgey");
    g.add_player(attacker);
    g.add_player(judge);

    force_turn(g, "Gov");
    attacker->set_coins(3);
    CHECK_THROWS_AS(attacker->sanction(*judge), NotEnoughCoinsException);
}

TEST_CASE("Spy abilities") {
    Game g;
    auto spy = std::make_shared<Spy>(g, "Spyman");
    auto target = std::make_shared<Governor>(g, "Gov");
    
    g.add_player(spy);
    g.add_player(target);

    target->set_coins(5);

    CHECK_NOTHROW(spy->peek_and_disable(*target));
    CHECK(g.is_arrest_blocked(target->get_name()));

   
}
TEST_CASE("Spy::peek_and_disable cannot be used twice in a round") {
    Game g;
    auto spy = std::make_shared<Spy>(g, "Spyman");
    auto target = std::make_shared<Governor>(g, "Gov");
    g.add_player(spy);
    g.add_player(target);

    CHECK_NOTHROW(spy->peek_and_disable(*target));
    CHECK_THROWS_AS(spy->peek_and_disable(*target), InvalidActionException);
}

TEST_CASE("Spy::peek_and_disable throws on self-target") {
    Game g;
    auto spy = std::make_shared<Spy>(g, "Spyman");
    g.add_player(spy);

    CHECK_THROWS_AS(spy->peek_and_disable(*spy), CannotTargetYourselfException);
}

TEST_CASE("Spy::peek_and_disable throws if target is already dead") {
    Game g;
    auto spy = std::make_shared<Spy>(g, "Spyman");
    auto target = std::make_shared<Governor>(g, "Gov");
    g.add_player(spy);
    g.add_player(target);

    target->set_active(false);
    CHECK_THROWS_AS(spy->peek_and_disable(*target), PlayerAlreadyDeadException);
}

TEST_CASE("Spy::peek_and_disable throws if arrest already blocked") {
    Game g;
    auto spy = std::make_shared<Spy>(g, "Spyman");
    auto target = std::make_shared<Governor>(g, "Gov");
    g.add_player(spy);
    g.add_player(target);

    g.block_arrest_for(target->get_name());
    CHECK_THROWS_AS(spy->peek_and_disable(*target), InvalidActionException);
}

TEST_CASE("Spy::peek_and_disable does not change turn") {
    Game g;
    auto spy = std::make_shared<Spy>(g, "Spyman");
    auto target = std::make_shared<Governor>(g, "Gov");
    g.add_player(spy);
    g.add_player(target);

    force_turn(g, "Spyman");
    std::string before = g.turn();
    spy->peek_and_disable(*target);
    std::string after = g.turn();

    CHECK(before == after);
}

