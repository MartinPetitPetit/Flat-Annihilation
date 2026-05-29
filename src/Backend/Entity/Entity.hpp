/*
 * Backend/Entity/Entity.hpp
 *
 * Rôle du fichier :
 * Declares the base Entity class shared by units and buildings.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Entity. Il sert de base commune pour les objets du jeu qui ont une position, une équipe et des points de vie.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#pragma once

#include <SDL2/SDL_image.h>
#include "../Coordinate/Coordinate.hpp"

class Entity
{
public:
    Entity();
    virtual ~Entity();

    virtual void update();

    void takeDamage(int amount);
    void heal(int amount);

    bool isAlive() const;
    bool isSelected() const;

    Coordinate  getPos() const;
    Coordinate& getPosRef();

    int getId() const;
    int getTeam() const;
    int getHealth() const;
    int getMaxHealth() const;

protected:
    int        id        { 0       };
    Coordinate position;
    int        health    { 0       };
    int        maxHealth { 0       };
    SDL_Texture* texture { nullptr };
    int        team      { -1      };
};
