#include "Resource.hpp"
#include "../ResourceManager/ResourceManager.hpp"
#include <cstdlib>

Resource::Resource(ResourceType type, int amount, int maxAmount)
    : type(type), amount(amount), maxAmount(maxAmount)
{
    if (type == wood) {
        texturePath = "assets/oak_tree.png";
    }
    else if (type == food) {
        if (amount > 0)
            texturePath = "assets/berry.png";
        else
            texturePath = "assets/bush.png";
    }
}

ResourceType Resource::getResourceType()
{
    return type;
}

void Resource::render(SDL_Renderer* renderer, SDL_Rect destination)
{
    if (texturePath.empty()) return;

    SDL_Texture* tex = ResourceManager::getInstance().getTexture(texturePath);
    if (!tex) return;

    SDL_RenderCopy(renderer, tex, nullptr, &destination);
}