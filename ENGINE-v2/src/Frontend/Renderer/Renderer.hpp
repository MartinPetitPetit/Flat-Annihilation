#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "../Window/Window.hpp"
#include "../../Backend/Map/Map.hpp"

class Renderer {
private:
    SDL_Renderer* sdlRenderer { nullptr };
    TTF_Font*     font        { nullptr };

    int scale   { 8 };
    int offsetX { 0 };
    int offsetY { 0 };

    /*
     * Internal drawing helpers.
     */
    unsigned int hashCell(int x, int y) const;

    void drawFilledCircle(int cx, int cy, int radius, SDL_Color color);

    /*
     * Tree drawing.
     */
    void drawTreeModel(const SDL_Rect& cellRect, WOOD_TYPE wood_type);
    void drawRoundTree(const SDL_Rect& cellRect);
    void drawPineTree(const SDL_Rect& cellRect);
    void drawSmallTree(const SDL_Rect& cellRect);

    /*
     * Bush / berry drawing.
     */
    void drawLeaf(int cx, int cy, int size, SDL_Color color);
    void drawBerry(int cx, int cy, int radius);
    void drawBerryBushModel(const SDL_Rect& cellRect, bool hasBerry);

    void drawLeafEllipse(
        int cx,
        int cy,
        int rx,
        int ry,
        SDL_Color fillColor,
        SDL_Color veinColor
    );

    void drawBerryCluster(int cx, int cy, int berryRadius);
    void drawBushModel(const SDL_Rect& cellRect, bool has_berry);

    /*
     * Structure / unit drawing.
     */
    void drawBaseModel(const SDL_Rect& cellRect);
    void drawCollectorModel(const SDL_Rect& cellRect);

public:
    Renderer(Window& window, const char* font_path);
    ~Renderer();

    void clear();
    void present();

    void drawTexture(SDL_Texture* tex, SDL_Rect* src, SDL_Rect* dst);
    void drawRect(const SDL_Rect& rect, SDL_Color color, bool filled);
    void drawMap(MAP& map, int MAP_W, int MAP_H, DISPLAY_OPTIONS& options);
    void drawText(const char* text, int x, int y);

    void applyZoom(int mouseX, int mouseY, int direction);

    void setOffset(int x, int y);

    int getOffsetX() const;
    int getOffsetY() const;
    int getScale() const;

    bool isValid() const;

    SDL_Renderer* getSDLRenderer() const;
    TTF_Font* getFont() const;
};
