#include "Entity.hpp"

Entity::Entity() {}
Entity::~Entity() {}

void Entity::update() {}

void Entity::takeDamage(int amount)
{
    health -= amount;
    if (health < 0) health = 0;
}

void Entity::heal(int amount)
{
    health += amount;
}

bool Entity::isAlive()    const { return health > 0; }
bool Entity::isSelected() const { return false; }

Coordinate  Entity::getPos()    const { return position;  }
Coordinate& Entity::getPosRef()       { return position;  }
int         Entity::getId()     const { return id;        }
int         Entity::getTeam()   const { return team;      }