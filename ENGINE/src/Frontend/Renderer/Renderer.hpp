#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../Window/Window.hpp"
#include "../../Backend/Map/Map.hpp"

class Renderer {
    SDL_Renderer* sdlRenderer { nullptr };
    TTF_Font*     font        { nullptr };
    int  scale   { 8 };
    int  offsetX { 0 };
    int  offsetY { 0 };
public:
    Renderer(Window& window, const char* font_path);
    ~Renderer();

    void clear();
    void present();
    void drawTexture(SDL_Texture* tex, SDL_Rect* src, SDL_Rect* dst);
    void drawRect(const SDL_Rect& rect, SDL_Color color, bool filled);
    void drawMap(MAP& map, int MAP_W, int MAP_H, DISPLAY_OPTIONS& options);
    void drawText(const char* text, int x, int y);
    void updateViewport(int w, int h);

    // Zoom
    void applyZoom(int mouseX, int mouseY, int direction);

    // Offset (drag)
    void setOffset(int x, int y);
    int  getOffsetX() const;
    int  getOffsetY() const;
    int  getScale()   const;

    bool          isValid()        const;
    SDL_Renderer* getSDLRenderer() const;
    TTF_Font*     getFont()        const;
};