#include "ResourceManager.hpp"
#include <iostream>

ResourceManager& ResourceManager::getInstance()
{
    static ResourceManager instance;
    return instance;
}

void ResourceManager::setRenderer(SDL_Renderer* r)
{
    renderer = r;
}

SDL_Texture* ResourceManager::getTexture(const std::string& path)
{
    auto it = cache.find(path);
    if (it != cache.end())
        return it->second;

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "ResourceManager: impossible de charger " << path
                  << " : " << IMG_GetError() << "\n";
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        std::cerr << "ResourceManager: impossible de créer texture pour "
                  << path << " : " << SDL_GetError() << "\n";
        return nullptr;
    }

    cache[path] = texture;
    return texture;
}

void ResourceManager::clear()
{
    for (auto& [path, tex] : cache)
        SDL_DestroyTexture(tex);
    cache.clear();
}

ResourceManager::~ResourceManager()
{
    clear();
}