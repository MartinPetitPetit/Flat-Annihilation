#pragma once
#include <SDL2/SDL_image.h>

enum TERRAIN {
    Plain,
    Montain,
    Lake,
    River,
    Bush,
    ravine
};

enum RESOURCE {
    None_Resource,
    tree,
    stone,
    gold,
    iron,
    Sapling,
    BushResource
};

enum UNIT {
    None_Unit,
    archer,
    MONK
};

enum WOOD_TYPE {
    No_Wood,
    Wood_A,
    Wood_B,
    Wood_C
};

class Cell
{
public:
    Cell();
    virtual ~Cell();

    TERRAIN   type_terrain;
    RESOURCE  type_resource;
    UNIT      type_unit;
    WOOD_TYPE wood_type;

    bool has_berry;
    bool walkable;
    bool occupied;

    int buildingID    { -1 };
    int buildingOwner { -1 };

    SDL_Texture* texture;
};