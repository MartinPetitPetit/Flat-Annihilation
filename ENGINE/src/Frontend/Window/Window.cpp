/*-SDL_Window* sdlWindow

-int width

-int height

+Window(title,w,h)

+getSDLWindow() : SDL_Window

+getSize() : Vector2i

+isValid() : bool

+~Window()*/

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
    SDL_SetWindowFullscreen(sdlWindow, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}