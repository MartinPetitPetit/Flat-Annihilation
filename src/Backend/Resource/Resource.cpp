/*
 * Backend/Resource/Resource.cpp
 *
 * Rôle du fichier :
 * Implements resource type data, available amount, gathering logic, sprite path selection, and rendering.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Resource. Il décrit les ressources récoltables et leur rendu.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#include "Resource.hpp"
#include "../ResourceManager/ResourceManager.hpp"

#include <algorithm>

Resource::Resource(ResourceType type, int amount, int maxAmount)
    : type(type), amount(amount), maxAmount(maxAmount)
{
    if (type == wood) {
        texturePath = "assets/terrain/resources/oak_tree.png";
    }
    else if (type == food) {
        if (amount > 0) {
            texturePath = "assets/terrain/resources/berry.png";
        }
        else {
            texturePath = "assets/terrain/resources/bush.png";
        }
    }
}

ResourceType Resource::getResourceType() const
{
    return type;
}

int Resource::getAmount() const
{
    return amount;
}

int Resource::getMaxAmount() const
{
    return maxAmount;
}

bool Resource::isEmpty() const
{
    return amount <= 0;
}

int Resource::gather(int requestedAmount)
{
    if (requestedAmount <= 0 || amount <= 0) {
        return 0;
    }

    int taken = std::min(requestedAmount, amount);
    amount -= taken;

    return taken;
}

void Resource::render(SDL_Renderer* renderer, SDL_Rect destination)
{
    if (texturePath.empty()) {
        return;
    }

    SDL_Texture* tex = ResourceManager::getInstance().getTexture(texturePath);

    if (!tex) {
        return;
    }

    SDL_RenderCopy(renderer, tex, nullptr, &destination);
}
