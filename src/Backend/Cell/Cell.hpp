#pragma once
#include <SDL2/SDL_image.h>
#include "../Resource/Resource.hpp"
Class Unit; // forward declaration to avoid circular dependency

enum TERRAIN {
    Plain,
    Montain,
    Lake,
    River,
    Bush,
    ravine
};


class Cell
{
public:
    Cell();
    virtual ~Cell();

    TERRAIN   type_terrain;
    Resource  resource;
    Unit      unit;
    
    bool walkable;
    bool occupied;
    int buildingID    { -1 };
    int buildingOwner { -1 };

    SDL_Texture* texture;
};