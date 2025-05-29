// Anksilae@gmail.com

#include "GUI.hpp"
#include "Game.hpp"

int main() {
    coup::Game game;
    coup::GUI gui(game,true); 
    gui.run();
    return 0;
}