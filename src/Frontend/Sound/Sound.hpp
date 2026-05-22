#pragma once
#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>

class Sound {
    std::unordered_map<std::string, Mix_Chunk*> samples;
    int volume { 64 }; // 0-128
private:
    Mix_Music* music { nullptr };

public:
    bool loadMusic(const std::string& path);
    void playMusic(int loops = -1); // -1 = boucle infinie
    void stopMusic();
    void setMusicVolume(int vol);
    Sound();
    ~Sound();

    bool load(const std::string& name, const std::string& path);
    void play(const std::string& name);
    void setVolume(int vol);
};