#include "Game/Game.hpp"

int main(int argc, char* argv[])
{
    Game* game = new Game();
    game->startGame();
    game->stopGame();
    delete game;
    return 0;
}