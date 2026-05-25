#pragma once
#include <vector>
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
    bool isAlive()    const;
    bool isSelected() const;

    Coordinate  getPos() const;
    Coordinate& getPosRef();

    int getId()   const;
    int getTeam() const;
    int getHealth() const { return health; }

protected:
    int        id       { 0       };
    Coordinate position;
    int        health   { 0       };
    SDL_Texture* texture{ nullptr };
    int        team     { -1      };
};