#include <iostream>

#include "Player.hpp"

Player::Player()
{
	std::cout << "nom du joueur : ? ";
	std::cin >> this->name;
}

Player::Player(int i)
{
	name = "IA" + std::to_string(i);
}

Player::~Player()
{
	std::cout << "destruction du joueur " << name << "\n";
}

const std::string Player::getName() const
{
	return name;
}