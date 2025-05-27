#include "GUI.hpp"
#include "Game.hpp"

int main() {
    coup::Game game;
    coup::GUI gui(game); 
    gui.run();
    return 0;
}