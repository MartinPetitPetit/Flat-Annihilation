/*
 * Backend/ResourceManager/ResourceManager.cpp
 *
 * Rôle du fichier :
 * Implements the singleton texture manager, path resolution, compatibility search, caching, and cleanup.
 *
 * Notes de lecture :
 * Ce fichier appartient au module ResourceManager. Il charge, résout, met en cache et libère les textures SDL.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#include "ResourceManager.hpp"

#include <SDL2/SDL_error.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    std::unordered_set<std::string> failedTexturePaths;
    std::unordered_map<std::string, std::string> resolvedTexturePaths;

    std::string filenameOnly(const std::string& path)
    {
        return std::filesystem::path(path).filename().string();
    }

    bool fileExists(const std::string& path)
    {
        std::error_code ec;
        return std::filesystem::is_regular_file(path, ec);
    }

    void addCandidate(std::vector<std::string>& candidates, const std::string& path)
    {
        if (path.empty()) {
            return;
        }

        if (std::find(candidates.begin(), candidates.end(), path) == candidates.end()) {
            candidates.push_back(path);
        }
    }

    void addCandidateWithPrefixes(
        std::vector<std::string>& candidates,
        const std::string& relativePath
    ) {
        addCandidate(candidates, relativePath);
        addCandidate(candidates, "src/" + relativePath);
        addCandidate(candidates, "../" + relativePath);
        addCandidate(candidates, "../../" + relativePath);
    }

    void addCommonResourceCandidates(
        std::vector<std::string>& candidates,
        const std::string& name
    ) {
        /*
         * Organisation actuelle des assets :
         * assets/terrain/resources/oak_tree.png
         * assets/terrain/resources/berry.png
         * assets/terrain/resources/bush.png
         */
        addCandidateWithPrefixes(candidates, "assets/terrain/resources/" + name);
        addCandidateWithPrefixes(candidates, "assets/resources/" + name);
        addCandidateWithPrefixes(candidates, "assets/resource/" + name);
        addCandidateWithPrefixes(candidates, "assets/images/" + name);
        addCandidateWithPrefixes(candidates, "assets/terrain/" + name);
        addCandidateWithPrefixes(candidates, "assets/terrain/plain/" + name);
        addCandidateWithPrefixes(candidates, "assets/" + name);
    }

    void addCompatibilityCandidates(
        std::vector<std::string>& candidates,
        const std::string& path
    ) {
        const std::string oldPrefix = "assets/resources/";
        const std::string newPrefix = "assets/terrain/resources/";

        if (path.rfind(oldPrefix, 0) == 0) {
            addCandidateWithPrefixes(candidates, newPrefix + path.substr(oldPrefix.size()));
        }

        const std::string singularPrefix = "assets/resource/";

        if (path.rfind(singularPrefix, 0) == 0) {
            addCandidateWithPrefixes(candidates, newPrefix + path.substr(singularPrefix.size()));
        }
    }

    std::string searchRecursivelyByFilename(
        const std::string& name,
        std::vector<std::string>& testedPaths
    ) {
        const std::vector<std::string> roots = {
            "assets",
            "src/assets",
            "../assets",
            "../../assets"
        };

        for (const std::string& root : roots) {
            std::error_code ec;

            if (!std::filesystem::exists(root, ec)) {
                continue;
            }

            for (const auto& entry : std::filesystem::recursive_directory_iterator(root, ec)) {
                if (ec) {
                    break;
                }

                if (!entry.is_regular_file(ec)) {
                    continue;
                }

                std::string candidate = entry.path().string();
                addCandidate(testedPaths, candidate);

                if (entry.path().filename().string() == name) {
                    return candidate;
                }
            }
        }

        return "";
    }

    std::string resolveTexturePath(
        const std::string& path,
        std::vector<std::string>& testedPaths
    ) {
        auto cached = resolvedTexturePaths.find(path);
        if (cached != resolvedTexturePaths.end()) {
            return cached->second;
        }

        std::vector<std::string> candidates;
        const std::string name = filenameOnly(path);

        addCandidateWithPrefixes(candidates, path);
        addCompatibilityCandidates(candidates, path);
        addCommonResourceCandidates(candidates, name);

        for (const std::string& candidate : candidates) {
            addCandidate(testedPaths, candidate);

            if (fileExists(candidate)) {
                resolvedTexturePaths[path] = candidate;
                return candidate;
            }
        }

        std::string recursiveCandidate = searchRecursivelyByFilename(name, testedPaths);

        if (!recursiveCandidate.empty()) {
            resolvedTexturePaths[path] = recursiveCandidate;
            return recursiveCandidate;
        }

        return path;
    }
}

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
    auto cachedTexture = cache.find(path);
    if (cachedTexture != cache.end()) {
        return cachedTexture->second;
    }

    if (failedTexturePaths.find(path) != failedTexturePaths.end()) {
        return nullptr;
    }

    if (renderer == nullptr) {
        std::cerr << "ResourceManager: renderer non initialisé pour " << path << "\n";
        failedTexturePaths.insert(path);
        return nullptr;
    }

    std::vector<std::string> testedPaths;
    std::string resolvedPath = resolveTexturePath(path, testedPaths);

    SDL_Surface* surface = IMG_Load(resolvedPath.c_str());

    if (!surface) {
        /*
         * On affiche l'erreur une seule fois par texture.
         * Sans cela, chaque arbre ou buisson répète le même message.
         */
        std::cerr << "ResourceManager: impossible de charger " << path << "\n";
        std::cerr << "Chemins testés:";

        for (const std::string& candidate : testedPaths) {
            std::cerr << " " << candidate;
        }

        std::cerr << "\nErreur SDL_image: " << IMG_GetError() << "\n";

        failedTexturePaths.insert(path);
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        std::cerr << "ResourceManager: impossible de créer texture pour "
                  << resolvedPath << " : " << SDL_GetError() << "\n";
        failedTexturePaths.insert(path);
        return nullptr;
    }

    cache[path] = texture;
    return texture;
}

void ResourceManager::clear()
{
    for (auto& [path, tex] : cache) {
        (void)path;

        if (tex != nullptr) {
            SDL_DestroyTexture(tex);
        }
    }

    cache.clear();
    failedTexturePaths.clear();
    resolvedTexturePaths.clear();
}

ResourceManager::~ResourceManager()
{
    clear();
}
