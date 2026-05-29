#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <unordered_map>

class ResourceManager {
public:
    static ResourceManager& getInstance();

    void        setRenderer(SDL_Renderer* renderer);
    SDL_Texture* getTexture(const std::string& path);
    void        clear();

private:
    ResourceManager() = default;
    ~ResourceManager();

    SDL_Renderer* renderer { nullptr };
    std::unordered_map<std::string, SDL_Texture*> cache;
};