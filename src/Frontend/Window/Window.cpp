/*
 * Frontend/Window/Window.cpp
 *
 * Rôle du fichier :
 * Creates and manages the SDL window, including size, fullscreen mode, resizing, and validation.
 *
 * Notes de lecture :
 * Ce module encapsule la fenêtre SDL et les options d'affichage.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#include "Window.hpp"

/*
 * Crée la fenêtre SDL avec les options initiales.
 */
Window::Window(const std::string& title, DISPLAY_OPTIONS opts)
    : options(opts)
{
    Uint32 flags = SDL_WINDOW_SHOWN;
    if (opts.fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    sdlWindow = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        opts.width, opts.height,
        flags
    );
}

/*
 * Détruit la fenêtre SDL si elle existe.
 */
Window::~Window()
{
    if (sdlWindow) SDL_DestroyWindow(sdlWindow);
}

SDL_Window*     Window::getSDLWindow() const { return sdlWindow; }
DISPLAY_OPTIONS Window::getOptions()   const { return options;   }
Vector2i        Window::getSize()      const { return { options.width, options.height }; }
bool            Window::isValid()      const { return sdlWindow != nullptr; }

/*
 * Active ou désactive le plein écran et ajuste la taille de fenêtre.
 */
void Window::setFullscreen(bool fs)
{
    options.fullscreen = fs;

    SDL_DisplayMode displayMode;
    if (this->options.fullscreen && SDL_GetDesktopDisplayMode(0, &displayMode) == 0) {
		resize(displayMode.w, displayMode.h);
    } else {
        SDL_Log("Erreur: %s", SDL_GetError());
    }

    SDL_SetWindowFullscreen(this->sdlWindow, (this->options.fullscreen) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);


}

/*
 * Redimensionne et recentre la fenêtre.
 */
void Window::resize(int width, int height)
{
    printf("Window::resize appelé : %dx%d\n", width, height); // ← debug
    this->options.width  = width;
    this->options.height = height;
    SDL_SetWindowSize(this->sdlWindow, this->options.width, this->options.height);
    SDL_SetWindowPosition(this->sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}