/*
 * Backend/Entity/Entity.cpp
 *
 * Rôle du fichier :
 * Implements common entity behavior such as health, damage, healing, identity, team, and position access.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Entity. Il sert de base commune pour les objets du jeu qui ont une position, une équipe et des points de vie.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#include "Entity.hpp"

Entity::Entity()
{
}

Entity::~Entity()
{
}

void Entity::update()
{
}

void Entity::takeDamage(int amount)
{
    if (amount <= 0 || health <= 0) {
        return;
    }

    health -= amount;

    if (health < 0) {
        health = 0;
    }
}

void Entity::heal(int amount)
{
    if (amount <= 0 || health <= 0) {
        return;
    }

    health += amount;

    if (maxHealth > 0 && health > maxHealth) {
        health = maxHealth;
    }
}

bool Entity::isAlive() const
{
    return health > 0;
}

bool Entity::isSelected() const
{
    return false;
}

Coordinate Entity::getPos() const
{
    return position;
}

Coordinate& Entity::getPosRef()
{
    return position;
}

int Entity::getId() const
{
    return id;
}

int Entity::getTeam() const
{
    return team;
}

int Entity::getHealth() const
{
    return health;
}

int Entity::getMaxHealth() const
{
    return maxHealth;
}
