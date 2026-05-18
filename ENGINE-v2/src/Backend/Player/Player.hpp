#pragma once

#include <string>
#include <vector>

class Entity;

class Player
{
public:
	Player();
	Player(int i);
	virtual ~Player();

	const std::string& getName() const;

private:
	int id { 0 };
	std::string name;

	std::vector<Entity*> entities;
};
