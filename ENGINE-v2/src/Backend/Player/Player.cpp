#ifndef PLAYER_CPP
#define PLAYER_CPP



#include <cstdio>
#include <iostream>
#include <new>

#include "Player.hpp"




Player::Player()
{
	char *buffer = new char;
    std::cout << "nom du joueur : ? ";
    std::cin >> buffer;
	this->name = buffer;
	delete buffer;
}



Player::Player(int i)
{
/* 	char *buffer = new char;
	sprintf(buffer, "IA%i", i);
	this->name = buffer;
	delete buffer; */
}



Player::~Player()
{
	std::cout << "destruction du joueur " << this->name << "\n";
	delete this->name;
}



const char *Player::getName() const
{
	return this->name;
}


#endif