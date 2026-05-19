#include "Renderer.hpp"

Renderer::Renderer(Window& window, const char* font_path)
{
    sdlRenderer = SDL_CreateRenderer(
        window.getSDLWindow(), -1,
        SDL_RENDERER_ACCELERATED
    );
    font = TTF_OpenFont(font_path, 18);
}

Renderer::~Renderer()
{
    if (font)        TTF_CloseFont(font);
    if (sdlRenderer) SDL_DestroyRenderer(sdlRenderer);
}

void Renderer::clear()
{
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
}

void Renderer::present()
{
    SDL_RenderPresent(sdlRenderer);
}

void Renderer::drawTexture(SDL_Texture* tex, SDL_Rect* src, SDL_Rect* dst)
{
    SDL_RenderCopy(sdlRenderer, tex, src, dst);
}

void Renderer::drawRect(const SDL_Rect& rect, SDL_Color color, bool filled)
{
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
    if (filled) SDL_RenderFillRect(sdlRenderer, &rect);
    else        SDL_RenderDrawRect(sdlRenderer, &rect);
}

void Renderer::drawText(const char* text, int x, int y)
{
    if (!font) return;
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Surface* surf = TTF_RenderText_Solid(font, text, white);
    SDL_Texture* tex  = SDL_CreateTextureFromSurface(sdlRenderer, surf);
    SDL_Rect dst = { x, y, surf->w, surf->h };
    SDL_RenderCopy(sdlRenderer, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void Renderer::drawMap(MAP& map, int MAP_W, int MAP_H, DISPLAY_OPTIONS& options)
{
    for (int x = 0; x < MAP_W; x++) {
        for (int y = 0; y < MAP_H; y++) {
            SDL_Rect cell;
            cell.x = offsetX + x * scale;
            cell.y = offsetY + y * scale;
            cell.w = scale;
            cell.h = scale;

            if (cell.x + cell.w < 0 || cell.x > options.width)  continue;
            if (cell.y + cell.h < 0 || cell.y > options.height) continue;

            SDL_Color color;
            switch (map[x][y].type_terrain) {
                case Plain:   color = {   0, 255,   0, 255 }; break;
                case Montain: color = { 139,  69,  19, 255 }; break;
                case Lake:    color = {   0,   0, 255, 255 }; break;
                case Bush:    color = {   0, 180,   0, 255 }; break;
                case ravine:  color = { 100, 100, 100, 255 }; break;
                default:      color = {   0,   0,   0, 255 }; break;
            }
            drawRect(cell, color, true);
        }
    }
}

void Renderer::applyZoom(int mouseX, int mouseY, int direction)
{
    int oldScale = scale;
    if (direction > 0 && scale < 64) scale++;
    else if (direction < 0 && scale > 2) scale--;
    offsetX = mouseX - (mouseX - offsetX) * scale / oldScale;
    offsetY = mouseY - (mouseY - offsetY) * scale / oldScale;
}
void Renderer::updateViewport(int w, int h) {
    SDL_RenderSetLogicalSize(sdlRenderer, 0, 0); // désactive le scaling automatique
    SDL_Rect vp = { 0, 0, w, h };
    SDL_RenderSetViewport(sdlRenderer, &vp);
}
void Renderer::setOffset(int x, int y) { offsetX = x; offsetY = y; }
int  Renderer::getOffsetX() const      { return offsetX; }
int  Renderer::getOffsetY() const      { return offsetY; }
int  Renderer::getScale()   const      { return scale;   }
bool Renderer::isValid()    const      { return sdlRenderer != nullptr; }
SDL_Renderer* Renderer::getSDLRenderer() const { return sdlRenderer; }
TTF_Font*     Renderer::getFont()        const { return font; }