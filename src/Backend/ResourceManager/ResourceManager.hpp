/*
 * Backend/ResourceManager/ResourceManager.hpp
 *
 * Rôle du fichier :
 * Declares the singleton resource manager used to load and reuse SDL textures.
 *
 * Notes de lecture :
 * Ce fichier appartient au module ResourceManager. Il charge, résout, met en cache et libère les textures SDL.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

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