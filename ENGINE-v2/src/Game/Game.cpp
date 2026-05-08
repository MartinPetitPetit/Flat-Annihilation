#ifndef GAME_CPP
#define GAME_CPP

#include "Game.hpp"
#include <iostream>



Game::Game()
{
	this->ptr_window = std::make_unique<Window>();
	this->ptr_renderer = std::make_unique<Renderer>();
	this->ptr_eventManager = std::make_unique<EventManager>();
	this->ptr_resourceManager = std::make_unique<ResourceManager>();
	this->ptr_uiManager = std::make_unique<UIManager>();
}



Game::~Game()
{
	std::cout << "destruction de Game\n";

	this->ptr_players.clear();
	this->ptr_map.reset();
}



void Game::startGame()
{
	this->ptr_players.push_back(std::make_unique<Player>());

	int maxX = 0, maxY = 0; 
	std::cout << "taille de la carte : maxX maxY = ";
	std::cin >> maxX >> maxY;

	this->ptr_map = std::make_unique<Map>(maxX, maxY);

	int nbPlayers = 0;

	std::cout << "combien de joueur IA = ? ";
	std::cin >> nbPlayers;

	for (int i = 0; i < nbPlayers; i++) {
		this->ptr_players.push_back(std::make_unique<Player>(i));
	}


	for (long unsigned int i = 0; i < this->ptr_players.size(); i++) {
		std::cout << "nom : " << this->ptr_players[i]->getName() << "\n";
	}


	this->running = true;
}



void Game::stopGame()
{
	// for (int i = 0; i < this->ptr_players.size(); i++) {
	// 	this->ptr_players.pop_back();
	// }

	this->ptr_players.clear();
	this->ptr_map.reset();

	this->running = false;
}



#endif