#pragma once
#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>

class Sound {
    std::unordered_map<std::string, Mix_Chunk*> samples;
    int volume { 64 }; // 0-128

public:
    Sound();
    ~Sound();

    bool load(const std::string& name, const std::string& path);
    void play(const std::string& name);
    void setVolume(int vol);
};