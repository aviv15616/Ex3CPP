// test_game.cpp - Unified full test coverage for Game class
// Anksilae@gmail.com

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Game.hpp"
#include "Player.hpp"
#include "Governor.hpp"
#include "Spy.hpp"
#include "Judge.hpp"
#include "Baron.hpp"
#include "General.hpp"
#include "Merchant.hpp"
#include "Exceptions.hpp"
#include <memory>
#include <string>
#include <vector>

using namespace coup;

class DummyPlayer : public Player {
public:
    DummyPlayer(Game& game_ref, const std::string& name)
        : Player(game_ref, name) {}
    std::string role() const override { return "Dummy"; }
};
TEST_CASE("add_player by role") {
    Game g;
    auto gov = g.add_player("G", "Governor");
    auto spy = g.add_player("S", "Spy");
    auto judge = g.add_player("J", "Judge");
    auto baron = g.add_player("B", "Baron");
    auto general = g.add_player("GN", "General");
    auto merchant = g.add_player("M", "Merchant");

    CHECK(gov->role() == "Governor");
    CHECK(spy->role() == "Spy");
    CHECK(judge->role() == "Judge");
    CHECK(baron->role() == "Baron");
    CHECK(general->role() == "General");
    CHECK(merchant->role() == "Merchant");
}

TEST_CASE("add_player unknown role") {
    Game g;
    CHECK_THROWS_AS(g.add_player("X", "InvalidRole"), InvalidActionException);
}
TEST_CASE("get_all_players_raw") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "A");
    g.add_player(p1);
    auto all = g.get_all_players_raw();
    CHECK(all.size() == 1);
    CHECK(all[0]->get_name() == "A");
}

TEST_CASE("get_last_actions") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "A");
    g.add_player(p1);
    g.perform_action("gather", "A");
    const auto& actions = g.get_last_actions();
    CHECK(actions.at("A") == "gather");
}
TEST_CASE("set/get current_turn_index") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "A");
    auto p2 = std::make_shared<DummyPlayer>(g, "B");
    g.add_player(p1);
    g.add_player(p2);

    g.set_current_turn_index(1);
    CHECK(g.get_current_turn_index() == 1);
}

TEST_CASE("set_current_turn_index") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "A");
    g.add_player(p1);
    CHECK_THROWS_AS(g.set_current_turn_index(-1), InvalidActionException);
    CHECK_THROWS_AS(g.set_current_turn_index(5), InvalidActionException);
}
TEST_CASE("Game::get_coup_pending_list") {
    Game game;
    game.add_player("Avi", "Spy");
    game.add_player("Dana", "Spy");

    game.add_to_coup("Avi", "Dana");
    const auto& list = game.get_coup_pending_list();
    CHECK(list.size() == 1);
    CHECK(list[0].first == "Avi");
    CHECK(list[0].second == "Dana");
}
TEST_CASE("Game::winner throws if game not over") {
    Game game;
    game.add_player("A", "Spy");
    game.add_player("B", "Spy");
    CHECK_THROWS_AS(game.winner(), GameNotOverException);
}

TEST_CASE("Game::winner returns correct player after elimination") {
    Game game;
    game.add_player("A", "Spy");
    game.add_player("B", "Spy");

    game.remove_player("B");

    // Simulate next_turn to trigger winner condition
    game.next_turn();
    CHECK(game.is_game_over());
    CHECK(game.winner() == "A");
}

TEST_CASE("Game::set_current_turn_index throws on invalid index") {
    Game game;
    game.add_player("A", "Spy");

    CHECK_THROWS_AS(game.set_current_turn_index(-1), InvalidActionException);
    CHECK_THROWS_AS(game.set_current_turn_index(5), InvalidActionException);
    CHECK_NOTHROW(game.set_current_turn_index(0));
}

TEST_CASE("Game::is_coup_pending_on") {
    Game game;
    game.add_player("A", "Spy");
    game.add_player("B", "Spy");

    CHECK(!game.is_coup_pending_on("B"));
    game.add_to_coup("A", "B");
    CHECK(game.is_coup_pending_on("B"));
}

TEST_CASE("Game::cancel_coup throws if target not found") {
    Game game;
    game.add_player("A", "Spy");
    game.add_player("B", "Spy");

    CHECK_THROWS_AS(game.cancel_coup("B"), InvalidActionException);
}

TEST_CASE("Game::cancel_coup restores player activity") {
    Game game;
    game.add_player("A", "Spy");
    game.add_player("B", "Spy");

    game.add_to_coup("A", "B");
    game.remove_player("B");
    CHECK(!game.get_player_by_name("B")->is_active());

    game.cancel_coup("B");
    CHECK(game.get_player_by_name("B")->is_active());
}
TEST_CASE("Game::end_game throws if game is already over") {
    Game game;
    game.add_player("Alice", "Spy");
    game.add_player("Bob", "Spy");

    game.end_game();
    CHECK_THROWS_AS(game.end_game(), GameAlreadyOverException);
}
TEST_CASE("Game::add_player throws on duplicate name") {
    Game game;
    game.add_player("Alice", "Spy");
    CHECK_THROWS_AS(game.add_player("Alice", "Spy"), DuplicatePlayerNameException);
}
TEST_CASE("Game::remove_player throws if player not found") {
    Game game;
    game.add_player("Alice", "Spy");
    CHECK_THROWS_AS(game.remove_player("Dana"), PlayerNotFoundException);
}
TEST_CASE("Game::next_turn erases coup if attacker reaches turn") {
    Game game;
    auto p1 = game.add_player("A", "Spy");
    auto p2 = game.add_player("B", "Spy");
    auto p3 = game.add_player("C", "Spy");
    p1->set_coins(7); // כדי לאפשר coup
    p1->coup(*p2);

    // נסובב את התור עד שמגיע ל-A, אז המחיקה תתרחש
    while (game.turn() != "A") {
        game.next_turn();
    }

    
    CHECK_FALSE(game.is_coup_pending_on("B"));
}


TEST_CASE("turn() and next_turn()") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "Alice");
    auto p2 = std::make_shared<DummyPlayer>(g, "Bob");
    auto p3 = std::make_shared<DummyPlayer>(g, "Charlie");

    g.add_player(p1);
    g.add_player(p2);
    g.add_player(p3);

    CHECK(g.turn() == "Alice");
    g.next_turn(); CHECK(g.turn() == "Bob");
    g.next_turn(); CHECK(g.turn() == "Charlie");
    g.next_turn(); CHECK(g.turn() == "Alice");
}

TEST_CASE("players()") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "Alice");
    auto p2 = std::make_shared<DummyPlayer>(g, "Bob");

    g.add_player(p1);
    g.add_player(p2);

    auto active = g.players();
    CHECK(active.size() == 2);
    p2->set_active(false);
    active = g.players();
    CHECK(active.size() == 1);
    CHECK(active[0] == "Alice");
}

TEST_CASE("winner()") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "Alice");
    auto p2 = std::make_shared<DummyPlayer>(g, "Bob");
    g.add_player(p1);
    g.add_player(p2);

    CHECK_THROWS_AS(g.winner(), GameNotOverException);
    p2->set_active(false);
    CHECK_NOTHROW(g.winner());
    CHECK(g.winner() == "Alice");
}



TEST_CASE("add_player") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "X");
    auto p2 = std::make_shared<DummyPlayer>(g, "X");
    g.add_player(p1);
    CHECK_THROWS_AS(g.add_player(p2), DuplicatePlayerNameException);;
}

TEST_CASE("remove_player and get_player_by_name") {
    Game g;
    auto p1 = std::make_shared<DummyPlayer>(g, "X");
    auto p2 = std::make_shared<DummyPlayer>(g, "Y");
    g.add_player(p1);
    g.add_player(p2);

    g.remove_player("X");
    CHECK_FALSE(p1->is_active());
    CHECK_THROWS_AS(g.get_player_by_name("T"), PlayerNotFoundException);
    CHECK_NOTHROW(g.get_player_by_name("Y"));
}

TEST_CASE("arrest logic and coup logic") {
    Game g;
    auto p = std::make_shared<DummyPlayer>(g, "Z");
    auto t = std::make_shared<DummyPlayer>(g, "Q");
    g.add_player(p);
    g.add_player(t);
    g.block_arrest_for("Z");
    CHECK(g.is_arrest_blocked("Z"));

    g.set_last_arrest_target("Z");
    CHECK(g.arrested_same_target("Z"));

    g.add_to_coup("Z", "Q");
    CHECK(g.is_coup_pending_on("Q"));
    CHECK_THROWS_AS(g.cancel_coup("Unknown"), InvalidActionException);
    g.cancel_coup("Q");
    CHECK_FALSE(g.is_coup_pending_on("Q"));
}

TEST_CASE("can still undo") {
    Game g;
    auto p = std::make_shared<DummyPlayer>(g, "A");
    g.add_player(p);
    g.perform_action("tax", "A");
    CHECK(g.can_undo_action("A", "tax"));
    g.cancel_last_action("A");
    CHECK_FALSE(g.can_undo_action("A", "tax"));

    g.perform_action("gather", "A");
    for (int i = 0; i < 6; ++i) {
        auto extra = std::make_shared<DummyPlayer>(g, "P" + std::to_string(i));
        g.add_player(extra);
    }
    for (int i = 0; i < 8; ++i) g.next_turn();
    CHECK_FALSE(g.can_still_undo("A"));
}

TEST_CASE("reset") {
    Game g;
    auto p = std::make_shared<DummyPlayer>(g, "Z");
    g.add_player(p);
    g.block_arrest_for("Z");
    g.set_last_arrest_target("Z");
    g.add_to_coup("Z", "Q");
    g.perform_action("tax", "Z");

    g.reset();
    CHECK_THROWS_AS(g.turn(), InvalidActionException);
    CHECK_THROWS_AS(g.get_player_by_name("Z"), PlayerNotFoundException);
    CHECK_FALSE(g.is_game_over());
}

TEST_CASE("game over prevents actions") {
    Game g;
    auto p = std::make_shared<DummyPlayer>(g, "A");
 
    g.end_game();
    CHECK(g.is_game_over());
    CHECK_THROWS_AS(g.add_player(std::make_shared<DummyPlayer>(g, "B")), GameAlreadyOverException);
}



TEST_CASE("next_turn resets sanction/arrest") {
    Game g;
    auto a = std::make_shared<DummyPlayer>(g, "X");
    auto b = std::make_shared<DummyPlayer>(g, "Y");
    g.add_player(a);
    g.add_player(b);
    a->set_coins(3);
    a->sanction(*b);
    b->disable_arrest();
    g.next_turn();
    CHECK_FALSE(b->is_sanctioned());
    CHECK_FALSE(b->is_arrest_disabled());
}
