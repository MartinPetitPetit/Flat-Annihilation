#include "Sound.hpp"
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <filesystem>

namespace
{
    std::string filenameOnly(const std::string& path)
    {
        return std::filesystem::path(path).filename().string();
    }

    std::vector<std::string> buildAudioCandidates(const std::string& path)
    {
        const std::string file = filenameOnly(path);

        std::vector<std::string> candidates;

        /*
         * On garde le chemin demandé en premier.
         * Ensuite, on teste uniquement les emplacements audio connus.
         * Cela évite qu'une recherche trop large charge un mauvais fichier.
         */
        candidates.push_back(path);
        candidates.push_back("assets/sounds/" + file);
        candidates.push_back("sounds/" + file);
        candidates.push_back("src/sounds/" + file);
        candidates.push_back("src/assets/sounds/" + file);

        /*
         * Cas où l'exécutable est lancé depuis build/bin.
         */
        candidates.push_back("../assets/sounds/" + file);
        candidates.push_back("../../assets/sounds/" + file);
        candidates.push_back("../sounds/" + file);
        candidates.push_back("../../sounds/" + file);

        return candidates;
    }

    Mix_Chunk* loadChunkFromKnownAudioPaths(const std::string& path)
    {
        std::vector<std::string> candidates = buildAudioCandidates(path);

        for (const std::string& candidate : candidates) {
            Mix_Chunk* chunk = Mix_LoadWAV(candidate.c_str());

            if (chunk != nullptr) {
                return chunk;
            }
        }

        std::cerr << "Erreur chargement " << path << ": " << Mix_GetError() << "\n";
        std::cerr << "Chemins testés:";

        for (const std::string& candidate : candidates) {
            std::cerr << " " << candidate;
        }

        std::cerr << "\n";
        return nullptr;
    }

    Mix_Music* loadMusicFromKnownAudioPaths(const std::string& path)
    {
        std::vector<std::string> candidates = buildAudioCandidates(path);

        for (const std::string& candidate : candidates) {
            Mix_Music* loadedMusic = Mix_LoadMUS(candidate.c_str());

            if (loadedMusic != nullptr) {
                return loadedMusic;
            }
        }

        std::cerr << "Erreur chargement musique " << path << ": " << Mix_GetError() << "\n";
        std::cerr << "Chemins testés:";

        for (const std::string& candidate : candidates) {
            std::cerr << " " << candidate;
        }

        std::cerr << "\n";
        return nullptr;
    }
}

Sound::Sound()
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        std::cerr << "SDL_mixer init error: " << Mix_GetError() << "\n";
}



bool Sound::load(const std::string& name, const std::string& path)
{
    Mix_Chunk* chunk = loadChunkFromKnownAudioPaths(path);
    if (!chunk) {
        return false;
    }
    samples[name] = chunk;
    return true;
}

void Sound::play(const std::string& name)
{
    auto it = samples.find(name);
    if (it == samples.end()) {
        std::cerr << "Sample inconnu: " << name << "\n";
        return;
    }
    Mix_VolumeChunk(it->second, volume);
    Mix_PlayChannel(-1, it->second, 0); // -1 = premier canal libre, 0 = pas de boucle
}

void Sound::setVolume(int vol)
{
    volume = std::clamp(vol, 0, 128);
}

Sound::~Sound()
{
    for (auto& [name, chunk] : samples) {
        (void)name;
        Mix_FreeChunk(chunk);
    }
    if (music) Mix_FreeMusic(music);
    Mix_CloseAudio();
}

bool Sound::loadMusic(const std::string& path)
{
    music = loadMusicFromKnownAudioPaths(path);
    if (!music) {
        return false;
    }
    return true;
}

void Sound::playMusic(int loops)
{
    if (!music) return;
    Mix_PlayMusic(music, loops); // -1 = infini
}

void Sound::stopMusic()
{
    Mix_HaltMusic();
}

void Sound::setMusicVolume(int vol)
{
    Mix_VolumeMusic(std::clamp(vol, 0, 256));
}
