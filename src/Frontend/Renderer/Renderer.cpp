#include "Renderer.hpp"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>


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

void Renderer::drawFilledCircle(int cx, int cy, int radius, SDL_Color color)
{
    if (radius <= 0) {
        return;
    }

    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);

    for (int dy = -radius; dy <= radius; dy++) {
        int dxLimit = static_cast<int>(
            std::sqrt(radius * radius - dy * dy)
        );

        SDL_RenderDrawLine(
            sdlRenderer,
            cx - dxLimit,
            cy + dy,
            cx + dxLimit,
            cy + dy
        );
    }
}

/*
 * ============================================================
 * UI
 * ============================================================
 */

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

void Renderer::drawMap(const MAP &map, int MAP_W, int MAP_H, DISPLAY_OPTIONS& options)
{

    int realWidth = static_cast<int>(map.size());

    if (realWidth == 0) {
        return;
    }

    int realHeight = static_cast<int>(map[0].size());

    if (realHeight == 0) {
        return;
    }

    if (MAP_W > realWidth) {
        MAP_W = realWidth;
    }

    if (MAP_H > realHeight) {
        MAP_H = realHeight;
    }

    for (int x = 0; x < MAP_W; x++) {
        for (int y = 0; y < MAP_H; y++) {

            SDL_Rect cell;
            cell.x = offsetX + x * scale;
            cell.y = offsetY + y * scale;
            cell.w = scale;
            cell.h = scale;

            if (cell.x + cell.w < 0 || cell.x > options.width) {
                continue;
            }

            if (cell.y + cell.h < 0 || cell.y > options.height) {
                continue;
            }

            /*
             * ==================================================
             * PART 1: TERRAIN
             * ==================================================
             */

            SDL_Color color;

            switch (map[x][y].type_terrain) {
                case Plain:
                    color = { 0, 255, 0, 255 };
                    break;

                case Montain:
                    color = { 139, 69, 19, 255 };
                    break;

                case Lake:
                    color = { 0, 0, 180, 255 };
                    break;

                case River:
                    color = { 0, 120, 255, 255 };
                    break;

                case Bush:
                    color = { 0, 180, 0, 255 };
                    break;

                case ravine:
                    if (scale <= 4) {
                        color = { 0, 0, 0, 255 };
                    }
                    else if (scale <= 8) {
                        color = { 25, 25, 25, 255 };
                    }
                    else {
                        color = { 50, 50, 50, 255 };
                    }
                    break;

                default:
                    color = { 0, 0, 0, 255 };
                    break;
            }

            drawRect(cell, color, true);

            /*
             * ==================================================
             * PART 2: RESOURCE OVER TERRAIN
             * ==================================================
             */

            SDL_Rect resourceRect;
            resourceRect.x = cell.x + cell.w / 4;
            resourceRect.y = cell.y + cell.h / 4;
            resourceRect.w = std::max(1, cell.w / 2);
            resourceRect.h = std::max(1, cell.h / 2);
			if (map[x][y].resource != nullptr) {
				switch (map[x][y].resource->getResourceType()) {
					case wood:

						map[x][y].resource->render(this->sdlRenderer, resourceRect);
						break;

					case stone:
						drawRect(resourceRect, { 120, 120, 120, 255 }, true);
						break;

					case gold:
						drawRect(resourceRect, { 255, 215, 0, 255 }, true);
						break;

					// case iron:
					//     drawRect(resourceRect, { 90, 90, 90, 255 }, true);
					//     break;

					// case Sapling:
					//     drawRect(resourceRect, { 0, 180, 60, 255 }, true);
					//     break;

					case food:
						map[x][y].resource->render(this->sdlRenderer, resourceRect);
						break;

					default:
						break;
				}
			}
            /*
             * ==================================================
             * PART 3: OPTIONAL GRID
             * ==================================================
             */

            if (scale >= 8) {
                drawRect(cell, { 0, 0, 0, 40 }, false);
            }
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