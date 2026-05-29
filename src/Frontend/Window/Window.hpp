/*
 * Frontend/Window/Window.hpp
 *
 * Rôle du fichier :
 * Declares display options, simple integer vector size type, and the SDL window wrapper.
 *
 * Notes de lecture :
 * Ce module encapsule la fenêtre SDL et les options d'affichage.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>

/* Petite structure utilitaire pour représenter une taille ou position entière. */
struct Vector2i { int x, y; };

/* Options d'affichage partagées entre la fenêtre, le renderer et les menus. */
struct DISPLAY_OPTIONS {
    int  width      { 800  };
    int  height     { 600  };
    bool fullscreen { false };
};

/*
 * Wrapper de fenêtre SDL.
 * Il garde les options d'affichage synchronisées avec l'objet SDL_Window.
 */
class Window {
    SDL_Window*     sdlWindow { nullptr };
    DISPLAY_OPTIONS options;
    /* Interface publique de création, interrogation et modification de la fenêtre. */
public:
    Window(const std::string& title, DISPLAY_OPTIONS opts);
    ~Window();
    SDL_Window*     getSDLWindow() const;
    DISPLAY_OPTIONS getOptions()   const;
    Vector2i        getSize()      const;
    bool            isValid()      const;
    void            setFullscreen(bool fs);
    void            resize(int width, int height);
};