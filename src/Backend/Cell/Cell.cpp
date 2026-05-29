#include "Cell.hpp"


Cell::Cell()
{
    type_terrain   = Plain;
    walkable       = true;
    occupied       = false;
    buildingID     = -1;
    buildingOwner  = -1;
	this->resource = nullptr;
	this->unit = nullptr;
}

Cell::~Cell() {
}


void Cell::setTexturePath(std::string texturePath){
    this->texturePath= texturePath;
}


void Cell::render(SDL_Renderer* renderer, SDL_Rect destination)
{
    if (texturePath.empty()) return;

    SDL_Texture* tex = ResourceManager::getInstance().getTexture(texturePath);
    if (!tex) return;

    SDL_RenderCopy(renderer, tex, nullptr, &destination);
}
