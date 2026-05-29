/*
 * Frontend/Sound/Sound.hpp
 *
 * Rôle du fichier :
 * Declares the sound manager for SDL_mixer samples, music, playback, and volume control.
 *
 * Notes de lecture :
 * Ce module encapsule SDL_mixer pour charger et jouer les sons du jeu.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#pragma once
#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>

/*
 * Gestionnaire audio simple basé sur SDL_mixer.
 * Il conserve les effets sonores nommés et une musique de fond.
 */
class Sound {
    std::unordered_map<std::string, Mix_Chunk*> samples;
    int volume { 64 }; // 0-128
private:
    Mix_Music* music { nullptr };

    /* Interface publique de chargement, lecture et réglage de volume. */
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