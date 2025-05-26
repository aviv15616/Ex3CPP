#include "Player.hpp"
#include "Governor.hpp"
#include "Spy.hpp"
#include "Baron.hpp"
#include "General.hpp"
#include "Judge.hpp"
#include "Merchant.hpp"
#include "Game.hpp"

#include <iostream>
#include <memory>
#include <vector>

using namespace std;
using namespace coup;

int main() {
    Game game;

    auto Governor1 = make_shared<Governor>(game, "Moshe");
    auto Spy2 = make_shared<Spy>(game, "Yossi");
    auto Baron3 = make_shared<Baron>(game, "Meirav");
    auto General4 = make_shared<General>(game, "Reut");
    auto Judge5 = make_shared<Judge>(game, "Gilad");
    auto Merchant6 = make_shared<Merchant>(game, "Tal");

    game.add_player(Governor1);
    game.add_player(Spy2);
    game.add_player(Baron3);
    game.add_player(General4);
    game.add_player(Judge5);
    game.add_player(Merchant6);

    game.print_state();

    // Round 1: gather for everyone
    Governor1->gather(); // 1
    Spy2->gather(); // 1
    Baron3->gather(); // 1
    General4->gather(); // 1
    Judge5->gather(); // 1
    Merchant6->gather(); // 1

    // Round 2: special role action (correct use)
    Governor1->tax();             // Governor special: +3
    try{
        Spy2->gather(); // Spy tries to gather (not his turn, should throw)
    } catch (const exception& e) {
        cerr << "Expected not your turn exception: " << e.what() << endl;
    }
    
    Spy2->disable_arrest(*General4); // Spy disables general's arrest
   
    Spy2->peek(*Governor1);         // Spy peeks governor, should print 3 coins
    Spy2->gather();          // Spy gathers, should be 2
    Baron3->gather();          // Baron gathers, should be 2
    try{
        General4->arrest(*Governor1); // General tries to arrest governor (illegal, the spy disabled it for this turn)
    } catch (const exception& e) {
        cerr << "Expected arrest error: " << e.what() << endl;
    }
    General4->gather();          // General gathers, should be 2
    Judge5->gather();          // Judge gathers, should be 2
    Merchant6->gather();          // Merchant gathers, should be 2

    // Round 3: 
    Governor1->gather();          // Governor should have 4 coins
    Spy2->gather();         // Spy should have 3 coins
    Baron3->gather();          // Baron should have 3 coins
    General4->arrest(*Governor1); // General arrests governor (General +1 coin, Governor -1 coin)
    Judge5->gather();         // Judge should have 3 coins now
    Merchant6->gather();          // Merchant should have 3 coins now

    // Round 4: use special actions (will throw some errors)
   
    Governor1->gather(); // baron should now have 4 coins
    Spy2->gather();
    Baron3->invest(); // baron should now have 6 coins
    General4->tax(); // General taxes, should have 5 coins now
    try {
        Judge5->undo_bribe(*Governor1); // should throw: no bribe done
    } catch (const exception& e) {
        cerr << "Expected undo_bribe error: " << e.what() << endl;
    }
    Governor1->undo_tax(*General4); // Governor undoes tax on General (General should have 3 coins now)
    Judge5->arrest(*Merchant6); // 
    try{
        Merchant6->gather(); // can't under sanction
    }
    catch (const exception& e) {
        cerr << "Expected gather error under sanction: " << e.what() << endl;
    }
    Merchant6->sanction(*Judge5); 
    
    // Round 5: more
    Governor1->tax(); // +3
    Spy2->gather();
    try {
        Baron3->arrest(*Merchant6); // should be blocked by spy
    } catch (const exception& e) {
        cerr << "Expected arrest blocked by spy: " << e.what() << endl;
    }
    General4->sanction(*Baron3); // general sanctions baron
    Judge5->gather();
    Merchant6->gather();

    // Round6:
    Governor1->tax();
    Spy2->gather();
    try{
    Baron3->gather();
    } catch (const exception& e) {
        cerr << "Expected gather error after coup: " << e.what() << endl;   
    }
    Baron3->invest(); // Baron invests, should have 6 coins now
    General4->gather();
    Judge5->gather();
    Merchant6->gather();
    Governor1->coup(*Judge5);

    game.print_state();

    // Round7:
    Governor1->gather(); 
    Spy2->gather();
    try{
        Baron3->gather(); // Baron can't gather has >=7 coins
    }
    catch (const exception& e) {
        cerr << "Expected gather error when coup is possible: " << e.what() << endl;   
    }
    Baron3->coup(*Judge5); // Baron coups Judge
    return 0;
}
