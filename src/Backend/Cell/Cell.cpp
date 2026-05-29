/*
 * Backend/Cell/Cell.cpp
 *
 * Rôle du fichier :
 * Initializes map cells and renders the cell terrain texture through the resource manager.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Cell. Il décrit une case de la carte et les éléments qui peuvent y être présents.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

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
