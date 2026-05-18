#include "Sound.hpp"
#include <iostream>

Sound::Sound()
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        std::cerr << "SDL_mixer init error: " << Mix_GetError() << "\n";
}

Sound::~Sound()
{
    for (auto& [name, chunk] : samples)
        Mix_FreeChunk(chunk);
    Mix_CloseAudio();
}

bool Sound::load(const std::string& name, const std::string& path)
{
    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) {
        std::cerr << "Erreur chargement " << path << ": " << Mix_GetError() << "\n";
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