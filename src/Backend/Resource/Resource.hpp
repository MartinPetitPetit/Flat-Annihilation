/*
 * Backend/Resource/Resource.hpp
 *
 * Rôle du fichier :
 * Declares resource types and the Resource class used by wood, food, and other map resources.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Resource. Il décrit les ressources récoltables et leur rendu.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

/*
 * Types de ressources utilisés par la carte et par le collecteur.
 */
enum ResourceType { food, wood, stone, gold };

class Resource {
public:
    Resource(ResourceType type, int amount, int maxAmount);
    virtual ~Resource() = default;

    void render(SDL_Renderer* renderer, SDL_Rect destination);

    ResourceType getResourceType() const;
    int getAmount() const;
    int getMaxAmount() const;
    bool isEmpty() const;

    /*
     * Retire une quantité disponible et retourne ce qui a vraiment été collecté.
     */
    int gather(int requestedAmount);

private:
    ResourceType type;
    int          amount;
    int          maxAmount;
    std::string  texturePath;
};
