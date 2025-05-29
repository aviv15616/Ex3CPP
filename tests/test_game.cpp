// Anksilae@gmail.com

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Game.hpp"
#include "Player.hpp"
#include "Exceptions.hpp"

#include <memory>
#include <string>
#include <vector>

using namespace coup;

// מחלקת עזר פשוטה – כי Player הוא מופשט
class DummyPlayer : public Player {
public:
    DummyPlayer(Game& game_ref, const std::string& name)
        : Player(game_ref, name) {}
    std::string role() const override { return "Dummy"; }
};

TEST_CASE("בדיקת turn() ו-next_turn()") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "Alice");
    auto p2 = std::make_shared<DummyPlayer>(g, "Bob");
    auto p3 = std::make_shared<DummyPlayer>(g, "Charlie");

    g.add_player(p1);
    g.add_player(p2);
    g.add_player(p3);

    CHECK(g.turn() == "Alice");
    g.next_turn();
    CHECK(g.turn() == "Bob");
    g.next_turn();
    CHECK(g.turn() == "Charlie");
    g.next_turn();
    CHECK(g.turn() == "Alice"); // חוזר להתחלה
}

TEST_CASE("players() מחזיר רק שחקנים פעילים") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "Alice");
    auto p2 = std::make_shared<DummyPlayer>(g, "Bob");

    g.add_player(p1);
    g.add_player(p2);

    auto active = g.players();
    CHECK(active.size() == 2);
    CHECK(active[0] == "Alice");
    CHECK(active[1] == "Bob");

    p2->set_active(false);
    active = g.players();
    CHECK(active.size() == 1);
    CHECK(active[0] == "Alice");
}

TEST_CASE("winner() זורק חריגה אם יש יותר משחקן אחד") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "Alice");
    auto p2 = std::make_shared<DummyPlayer>(g, "Bob");

    g.add_player(p1);
    g.add_player(p2);

    CHECK_THROWS_AS(g.winner(), GameNotOverException);
}

TEST_CASE("winner() מחזיר את שם השחקן היחיד") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "Alice");
    auto p2 = std::make_shared<DummyPlayer>(g, "Bob");

    g.add_player(p1);
    g.add_player(p2);

    p2->set_active(false);
    CHECK_NOTHROW(g.winner());
    CHECK(g.winner() == "Alice");
}
TEST_CASE("הפיכה נמחקת בסיום תור השחקן שביצע אותה") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "A");
    auto p2 = std::make_shared<DummyPlayer>(g, "B");
    g.add_player(p1);
    g.add_player(p2);

    // סימולציה של מצב שבו השחקן הפעיל ביצע הפיכה
    g.mark_coup_pending("A", "B");
    CHECK(g.is_coup_pending_on("B")); // ודא שהוגדר

    g.next_turn(); // A הוא prev => צריך לאפס

    // ודא שה־coup נמחקה
    CHECK_FALSE(g.is_coup_pending_on("B"));
}


TEST_CASE("add_player() עם שם כפול יזרוק DuplicatePlayerNameException") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "Alice");
    auto p2 = std::make_shared<DummyPlayer>(g, "Alice"); // שם כפול

    g.add_player(p1);
    CHECK_THROWS_AS(g.add_player(p2), DuplicatePlayerNameException);
}

TEST_CASE("turn() זורק אם אין שחקנים") {
    Game g;
    CHECK_THROWS_AS(g.turn(), InvalidActionException);
}

TEST_CASE("remove_player ו-get_player_by_name") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "X");
    auto p2 = std::make_shared<DummyPlayer>(g, "Y");
    g.add_player(p1);
    g.add_player(p2);

    g.remove_player("X");
    CHECK_FALSE(p1->is_active());
    CHECK_THROWS_AS(g.get_player_by_name("X"), PlayerNotFoundException);
    CHECK_NOTHROW(g.get_player_by_name("Y"));
}

TEST_CASE("block_arrest_for + is_arrest_blocked") {
    Game g;
    g.block_arrest_for("Avi");
    CHECK(g.is_arrest_blocked("Avi"));
    CHECK_FALSE(g.is_arrest_blocked("Dana"));
}

TEST_CASE("set_last_arrest_target + arrested_same_target_last_turn") {
    Game g;
    g.set_last_arrest_target("Avi", "Dana");
    CHECK(g.arrested_same_target_last_turn("Avi", "Dana"));
    CHECK_FALSE(g.arrested_same_target_last_turn("Avi", "SomeoneElse"));
    CHECK_FALSE(g.arrested_same_target_last_turn("Other", "Dana"));
}

TEST_CASE("mark_coup_pending, is_coup_pending_on, get_coup_attacker, cancel_coup") {
    Game g;
    g.mark_coup_pending("A", "B");
    CHECK(g.is_coup_pending_on("B"));
    CHECK(g.get_coup_attacker("B") == "A");
    g.cancel_coup();
    CHECK_FALSE(g.is_coup_pending_on("B"));
}

TEST_CASE("perform_action, can_undo_action, cancel_last_action") {
    Game g;
    auto p = std::make_shared<DummyPlayer>(g, "P");
    g.add_player(p);
    g.perform_action("tax", "P");
    CHECK(g.can_undo_action("P", "tax"));
    g.cancel_last_action("P");
    CHECK_FALSE(g.can_undo_action("P", "tax"));
}

TEST_CASE("reset() מאפס את כל מצב המשחק") {
    Game g;
    auto p = std::make_shared<DummyPlayer>(g, "Z");
    g.add_player(p);
    g.block_arrest_for("Z");
    g.set_last_arrest_target("Z", "Q");
    g.mark_coup_pending("Z", "Q");
    g.perform_action("tax", "Z");

    g.reset();

    CHECK_THROWS_AS(g.turn(), InvalidActionException);
    CHECK_FALSE(g.is_arrest_blocked("Z"));
    CHECK_FALSE(g.is_coup_pending_on("Q"));
    CHECK_FALSE(g.arrested_same_target_last_turn("Z", "Q"));
    CHECK_FALSE(g.can_undo_action("Z", "tax"));
}

TEST_CASE("is_game_over מחזיר false כברירת מחדל") {
    Game g;
    CHECK_FALSE(g.is_game_over());
}
TEST_CASE("add_player - game_over מונע הוספת שחקנים") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "A");
    g.add_player(p1);

    g.end_game(); // סימון המשחק כסגור

    auto p2 = std::make_shared<DummyPlayer>(g, "B");
    CHECK_THROWS_AS(g.add_player(p2), GameAlreadyOverException);
}

TEST_CASE("perform_action - זורק חריגה אם זה לא תור השחקן") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "A");
    auto p2 = std::make_shared<DummyPlayer>(g, "B");
    g.add_player(p1);
    g.add_player(p2);

    CHECK_THROWS_AS(g.perform_action("tax", "B"), InvalidActionException);
}

TEST_CASE("get_coup_attacker - זורק חריגה אם אין הפיכה על המטרה") {
    Game g;
    CHECK_THROWS_AS(g.get_coup_attacker("NotFound"), InvalidActionException);
}

TEST_CASE("remove_player - זורק חריגה אם לא קיים") {
    Game g;
    auto p = std::make_shared<DummyPlayer>(g, "Y");
    g.add_player(p);

    CHECK_THROWS_AS(g.remove_player("ZZZ"), PlayerNotFoundException);
}