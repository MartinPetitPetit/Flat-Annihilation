#include "Game/Game.hpp"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    Game game;
    game.startGame();
    game.stopGame();

    return 0;
}
