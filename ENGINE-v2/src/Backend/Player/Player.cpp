#include "Player.hpp"

#include <iostream>
#include <string>

Player::Player()
{
	std::cout << "nom du joueur : ? ";
	std::cin >> name;
}

Player::Player(int i)
{
	name = "IA" + std::to_string(i);
}

Player::~Player()
{
	std::cout << "destruction du joueur " << name << "\n";
}

const std::string& Player::getName() const
{
	return name;
}
