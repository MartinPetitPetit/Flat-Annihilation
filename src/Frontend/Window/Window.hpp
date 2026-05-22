#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>

struct Vector2i { int x, y; };

struct DISPLAY_OPTIONS {
    int  width      { 800  };
    int  height     { 600  };
    bool fullscreen { false };
};

class Window {
    SDL_Window*     sdlWindow { nullptr };
    DISPLAY_OPTIONS options;
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