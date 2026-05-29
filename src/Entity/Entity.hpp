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
