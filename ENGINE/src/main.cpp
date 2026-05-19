#include "Game/Game.hpp"

int main()
{
    Game* game = new Game();
    game->startGame();
    game->stopGame();
    delete game;
    return 0;
}