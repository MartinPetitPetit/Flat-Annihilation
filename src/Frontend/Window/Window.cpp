#include "Window.hpp"

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

Window::~Window()
{
    if (sdlWindow) SDL_DestroyWindow(sdlWindow);
}

SDL_Window*     Window::getSDLWindow() const { return sdlWindow; }
DISPLAY_OPTIONS Window::getOptions()   const { return options;   }
Vector2i        Window::getSize()      const { return { options.width, options.height }; }
bool            Window::isValid()      const { return sdlWindow != nullptr; }

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

void Window::resize(int width, int height)
{
    printf("Window::resize appelé : %dx%d\n", width, height); // ← debug
    this->options.width  = width;
    this->options.height = height;
    SDL_SetWindowSize(this->sdlWindow, this->options.width, this->options.height);
    SDL_SetWindowPosition(this->sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}