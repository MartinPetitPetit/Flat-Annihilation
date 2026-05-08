
#include "main.hpp"
#include "Game/Game.hpp"

#include <iostream>
#include <new>



int main()
{


	std::cout << "début du test\n";

	Game *testGame = new Game();

	testGame->startGame();
	std::cout << "fin de la partie\n";
	testGame->stopGame();

	delete testGame;


	return 1;
}