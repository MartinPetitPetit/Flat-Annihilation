#pragma once
#include <SDL2/SDL_image.h>
#include "../Resource/Resource.hpp"
#include <string>
#include "ResourceManager.hpp"
class Unit; // forward declaration to avoid circular dependency

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
        void render(SDL_Renderer* renderer, SDL_Rect destination);
        void setTexturePath(std::string texturePath);

        TERRAIN   type_terrain;
        Resource  *resource;
        Unit      *unit;

        bool walkable;
        bool occupied;
        int buildingID    { -1 };
        int buildingOwner { -1 };

    private:

        std::string  texturePath;
};
