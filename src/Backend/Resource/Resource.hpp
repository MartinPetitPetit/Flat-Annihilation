#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

enum ResourceType { food, wood, stone, gold };

class Resource {
public:
    Resource(ResourceType type, int amount, int maxAmount);
    virtual ~Resource() = default;

    void render(SDL_Renderer* renderer, SDL_Rect destination);
    ResourceType getResourceType();

private:
    ResourceType type;
    int          amount;
    int          maxAmount;
    std::string  texturePath;
};