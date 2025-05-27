    #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
    #include "doctest.h"
    #include "Player.hpp"
    #include "Game.hpp"
    #include "Exceptions.hpp"
    #include <memory>
    #include <string>

    using namespace coup;
    namespace coup {
    class TestPlayer : public Player {
    public:
        TestPlayer(Game& game, const std::string& name, const std::string& role_name = "TestRole")
            : Player(game, name), fake_role(role_name) {}

        std::string role() const override { return fake_role; }

        void on_arrest() override { arrested_called = true; }
        void on_sanction() override { sanctioned_called = true; }
        void on_turn_start() override { turn_start_called = true; }

        bool arrested_called = false;
        bool sanctioned_called = false;
        bool turn_start_called = false;

        void set_fake_role(const std::string& r) { fake_role = r; }

    private:
        std::string fake_role;
    };

    class TestGame : public Game {
    public:
        void force_turn(const std::string& name) {
            for (size_t i = 0; i < players().size() + 5; ++i) {
                if (turn() == name) return;
                next_turn();
            }
        }

        void simulate_last_action(const std::string& player, const std::string& action) {
            perform_action(action, player);
        }

        void simulate_arrest_block(const std::string& player) {
            block_arrest_for(player);
        }

        void clear_last_action(const std::string& player) {
            cancel_last_action(player);
        }

    void unblock_arrest_for(const std::string& name) {
        arrest_blocked_players.erase(name);
    }

    };
    }
    using namespace coup;

    TEST_CASE("Full coverage for Player.cpp") {
        TestGame g;
        auto p1 = std::make_shared<TestPlayer>(g, "Alice");
        auto p2 = std::make_shared<TestPlayer>(g, "Bob");

        g.add_player(p1);
        g.add_player(p2);

        SUBCASE("Basics: name, coins, set_active") {
            CHECK(p1->get_name() == "Alice");
            CHECK(p1->coins() == 0);
            p1->set_coins(5);
            CHECK(p1->coins() == 5);
            p1->set_active(false);
            CHECK_FALSE(p1->is_active());
        }

        SUBCASE("set_coins negative throws") {
            CHECK_THROWS_AS(p1->set_coins(-1), InvalidActionException);
        }

        SUBCASE("gather success and fail cases") {
            g.force_turn("Alice");
            p1->set_coins(0);
            p1->gather();
            CHECK(p1->coins() == 1);

            g.force_turn("Alice");
            p1->set_coins(10);
            CHECK_THROWS_AS(p1->gather(), InvalidActionException);

            g.force_turn("Bob");
            CHECK_THROWS_AS(p1->gather(), NotYourTurnException);
        }

        
        SUBCASE("arrest edge cases") {
            g.force_turn("Alice");
            p1->set_coins(5);
            p2->set_coins(1);

            CHECK_THROWS_AS(p1->arrest(*p1), CannotTargetYourselfException);
            p2->set_active(false);
            CHECK_THROWS_AS(p1->arrest(*p2), PlayerAlreadyDeadException);
            p2->set_active(true);
            p2->set_coins(0);
            CHECK_THROWS_AS(p1->arrest(*p2), InvalidActionException);

            g.simulate_arrest_block("Alice");
            p2->set_coins(2);
            CHECK_THROWS_AS(p1->arrest(*p2), InvalidActionException);

            g.unblock_arrest_for("Alice");
            g.force_turn("Alice");
            g.clear_last_action("Alice");
            p1->set_coins(5);
            p2->set_coins(2);
            p1->arrest(*p2);
            CHECK(p1->coins() == 6);
            CHECK(p2->coins() == 1);
            CHECK(p2->arrested_called);
        }

        SUBCASE("sanction behavior") {
            g.force_turn("Alice");
            p1->set_coins(2);
            CHECK_THROWS_AS(p1->sanction(*p2), NotEnoughCoinsException);

            g.force_turn("Alice");
            p1->set_coins(3);
            CHECK_NOTHROW(p1->sanction(*p2));
            CHECK(p1->coins() == 0);
            CHECK(p2->sanctioned_called);
        }

        SUBCASE("coup behavior") {
            g.force_turn("Alice");
            p1->set_coins(6);
            CHECK_THROWS_AS(p1->coup(*p2), NotEnoughCoinsException);

            g.force_turn("Alice");
            p1->set_coins(7);
            CHECK_NOTHROW(p1->coup(*p2));
            CHECK_FALSE(p2->is_active());
            CHECK(p1->coins() == 0);
        }

        SUBCASE("ensure_coup_required logic") {
            p1->set_coins(10);
            CHECK_THROWS_AS(p1->gather(), InvalidActionException);
        }


        SUBCASE("on_turn_start is called") {
            g.force_turn("Bob");    // ודא ש-Alice לא בתורה
            g.force_turn("Alice");  // מעבר תור => on_turn_start תופעל
            CHECK(p1->turn_start_called);  // עכשיו זה יעבור
        }
        SUBCASE("tax fails if not your turn") {
        g.force_turn("Bob"); // תוודא שזה לא Alice
        CHECK_THROWS_AS(p1->tax(), NotYourTurnException);
    }
    SUBCASE("arrest fails if repeated on same player") {
        g.force_turn("Alice");
        p1->set_coins(5);
        p2->set_coins(2);
        g.clear_last_action("Alice");
        g.unblock_arrest_for("Alice");

        p1->arrest(*p2);  // arrest ראשון – תקין
        g.force_turn("Alice");
        p2->set_coins(2); // מחזיר לו כסף כדי לוודא שעדיין תקף
        CHECK_THROWS_AS(p1->arrest(*p2), InvalidActionException);  // אמור להיכשל
    }
    SUBCASE("sanction fails if not your turn") {
        g.force_turn("Bob");
        p1->set_coins(5);
        CHECK_THROWS_AS(p1->sanction(*p2), NotYourTurnException);
    }
    SUBCASE("arrest throws when not your turn") {
        g.force_turn("Bob");  // לא תור של Alice
        p1->set_coins(5);
        p2->set_coins(2);
        CHECK_THROWS_AS(p1->arrest(*p2), NotYourTurnException);
    }

    SUBCASE("coup throws when not your turn") {
        g.force_turn("Bob");  // לא תור של Alice
        p1->set_coins(7);
        p2->set_active(true);
        CHECK_THROWS_AS(p1->coup(*p2), NotYourTurnException);
    }
    SUBCASE("All coin modifications never go negative") {
        CHECK_THROWS_AS(p1->set_coins(-100), InvalidActionException);
        p1->set_coins(2);
        CHECK_THROWS_AS(p1->set_coins(p1->coins() - 5), InvalidActionException);
    }
    SUBCASE("base class tax and bribe get called") {
            class PlainPlayer : public Player {
            public:
                PlainPlayer(Game& game, const std::string& name) : Player(game, name) {}
                std::string role() const override { return "Plain"; }
            };

            auto pp = std::make_shared<PlainPlayer>(g, "Plain");
            g.add_player(pp);

            g.force_turn("Plain");
            pp->set_coins(5);
            CHECK_NOTHROW(pp->tax());
            CHECK(pp->coins() == 7);

            g.force_turn("Plain");
            CHECK_NOTHROW(pp->bribe());
            CHECK(pp->coins() == 3);
        }

        SUBCASE("undo_tax and undo_bribe do nothing by default") {
            class Dummy : public Player {
            public:
                Dummy(Game& game, const std::string& name) : Player(game, name) {}
                std::string role() const override { return "Dummy"; }
            };

            auto d1 = std::make_shared<Dummy>(g, "D1");
            auto d2 = std::make_shared<Dummy>(g, "D2");
            g.add_player(d1);
            g.add_player(d2);

            CHECK_NOTHROW(d1->undo_tax(*d2));
            CHECK_NOTHROW(d1->undo_bribe(*d2));
        }

        SUBCASE("default on_arrest and on_sanction get called") {
            class PlainPlayer : public Player {
            public:
                PlainPlayer(Game& game, const std::string& name) : Player(game, name) {}
                std::string role() const override { return "Plain"; }
            };

            auto pp1 = std::make_shared<PlainPlayer>(g, "PP1");
            auto pp2 = std::make_shared<PlainPlayer>(g, "PP2");
            g.add_player(pp1);
            g.add_player(pp2);

            g.force_turn("PP1");
            pp1->set_coins(5);
            pp2->set_coins(2);
            CHECK_NOTHROW(pp1->arrest(*pp2));

            g.force_turn("PP1");
            pp1->set_coins(3);
            pp2->set_active(true);
            CHECK_NOTHROW(pp1->sanction(*pp2));
        }


    }
