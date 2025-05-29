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

// פונקציית עזר לוודא שהתור עובר לשחקן הרצוי
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
    gov->set_coins(0);
    gov2->set_coins(2);
    g.perform_action("tax", gov2->get_name());  // תור עבר אוטומטית ל־Gov

    // עכשיו זה תורו של Gov
    CHECK_NOTHROW(gov->undo_tax(*gov2));
    CHECK(gov->coins() == 2);
    CHECK(gov2->coins() == 0);

    g.perform_action("tax", gov2->get_name());  // תור עבר שוב ל־Gov
    gov2->set_coins(1);  // אין מספיק כדי להחזיר
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

    g.mark_coup_pending("Someone", victim->get_name());

    gen->set_coins(5);
    CHECK_NOTHROW(gen->block_coup(*victim));
    CHECK_FALSE(g.is_coup_pending_on(victim->get_name()));

    gen->set_coins(0);
    gen->on_arrest();
    CHECK(gen->coins() == 1);
}
TEST_CASE("General::block_coup throws if not enough coins") {
    Game g;
    auto general = std::make_shared<General>(g, "Gen");
    auto victim = std::make_shared<Spy>(g, "Victim");
    g.add_player(general);
    g.add_player(victim);
    g.mark_coup_pending("Someone", victim->get_name());
    general->set_coins(4);  // פחות מ־5
    CHECK_THROWS_AS(general->block_coup(*victim), NotEnoughCoinsException);
}
TEST_CASE("General::block_coup throws if no coup pending") {
    Game g;
    auto general = std::make_shared<General>(g, "Gen");
    auto victim = std::make_shared<Spy>(g, "Victim");
    g.add_player(general);
    g.add_player(victim);
    general->set_coins(5);
    CHECK_THROWS_AS(general->block_coup(*victim), InvalidActionException);
}

TEST_CASE("Judge undo_bribe and sanction logic") {
    Game g;
    auto judge = std::make_shared<Judge>(g, "Judge");
    auto bribing_player = std::make_shared<Spy>(g, "Spyyy");
    g.add_player(judge);
    g.add_player(bribing_player);

    bribing_player->set_coins(4);
    force_turn(g, "Spyyy");  // חובה! כדי שהוא בתור
    g.perform_action("bribe", bribing_player->get_name());

    force_turn(g, "Judge");
    CHECK_NOTHROW(judge->undo_bribe(*bribing_player));
    CHECK(judge->coins() == 4);
    CHECK(bribing_player->coins() == 0);

    SUBCASE("fails if bribing player has insufficient coins") {
        bribing_player->set_coins(4);
        force_turn(g, "Spyyy");  // חובה!
        g.perform_action("bribe", bribing_player->get_name());

        bribing_player->set_coins(2);  // אין מספיק
        force_turn(g, "Judge");
        CHECK_THROWS_AS(judge->undo_bribe(*bribing_player), NotEnoughCoinsException);
    }

    SUBCASE("fails if cannot undo") {
        bribing_player->set_coins(4);
        force_turn(g, "Judge");
        CHECK_THROWS_AS(judge->undo_bribe(*bribing_player), UndoNotAllowed);
    }
}


TEST_CASE("Merchant turn start bonus and arrest penalty") {
    Game g;
    auto m = std::make_shared<Merchant>(g, "Merch");
    g.add_player(m);

    m->set_coins(3);
    m->on_turn_start();  // bonus
    CHECK(m->coins() == 4);

    m->set_coins(2);
    CHECK_NOTHROW(m->on_arrest());
    CHECK(m->coins() == 0);

    m->set_coins(1);
    CHECK_THROWS_AS(m->on_arrest(), NotEnoughCoinsException);
}

TEST_CASE("Player sanctions Judge — extra coin is deducted") {
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

    SUBCASE("peek shows coins") {
        target->set_coins(5);
        CHECK_NOTHROW(spy->peek(*target));
    }

    SUBCASE("disable_arrest works") {
        CHECK_NOTHROW(spy->disable_arrest(*target));
        CHECK(g.is_arrest_blocked(target->get_name()));
    }

    SUBCASE("disable_arrest throws if target dead") {
        target->set_active(false);
        CHECK_THROWS_AS(spy->disable_arrest(*target), PlayerAlreadyDeadException);
    }

    SUBCASE("disable_arrest does not affect turn") {
        std::string before = g.turn();
        spy->disable_arrest(*target);
        std::string after = g.turn();
        CHECK(before == after);
    }
}
