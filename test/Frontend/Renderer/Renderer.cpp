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
 * BUSH / BERRY DRAWING
 * ============================================================
 */

void Renderer::drawLeaf(int cx, int cy, int size, SDL_Color color)
{
    drawFilledCircle(cx, cy, size, color);
    drawFilledCircle(cx + size / 2, cy - size / 3, size / 2, color);
    drawFilledCircle(cx - size / 2, cy + size / 3, size / 2, color);

    SDL_SetRenderDrawColor(sdlRenderer, 20, 90, 20, 255);

    SDL_RenderDrawLine(
        sdlRenderer,
        cx - size,
        cy + size,
        cx + size,
        cy - size
    );
}

void Renderer::drawBerry(int cx, int cy, int radius)
{
    drawFilledCircle(cx, cy, radius, { 190, 20, 35, 255 });
    drawFilledCircle(cx - radius / 3, cy - radius / 3, radius / 3, { 255, 130, 130, 255 });

    SDL_SetRenderDrawColor(sdlRenderer, 80, 20, 20, 255);
    SDL_RenderDrawPoint(sdlRenderer, cx + radius / 2, cy + radius / 2);
}

void Renderer::drawBerryBushModel(const SDL_Rect& cell, bool hasBerry)
{
    int cx = cell.x + cell.w / 2;
    int cy = cell.y + cell.h / 2;

    int s = cell.w;

    if (cell.h < s) {
        s = cell.h;
    }

    if (s < 4) {
        return;
    }

    int leafSize = std::max(2, s / 5);
    int berryRadius = std::max(2, s / 8);

    /*
     * Branches.
     */
    SDL_SetRenderDrawColor(sdlRenderer, 80, 50, 25, 255);

    SDL_RenderDrawLine(sdlRenderer, cx, cy, cx - s / 4, cy - s / 5);
    SDL_RenderDrawLine(sdlRenderer, cx, cy, cx + s / 4, cy - s / 5);
    SDL_RenderDrawLine(sdlRenderer, cx, cy, cx - s / 6, cy + s / 4);
    SDL_RenderDrawLine(sdlRenderer, cx, cy, cx + s / 6, cy + s / 4);

    /*
     * Leaves.
     */
    drawLeaf(cx - s / 4, cy - s / 4, leafSize, { 70, 160, 60, 255 });
    drawLeaf(cx + s / 4, cy - s / 4, leafSize, { 90, 180, 65, 255 });
    drawLeaf(cx,       cy - s / 3, leafSize, { 60, 140, 55, 255 });

    if (!hasBerry) {
        drawLeaf(cx - s / 8, cy + s / 8, leafSize, { 50, 130, 45, 255 });
        drawLeaf(cx + s / 8, cy + s / 8, leafSize, { 70, 150, 50, 255 });
        return;
    }

    /*
     * Berries.
     */
    drawBerry(cx - s / 5,  cy + s / 8, berryRadius);
    drawBerry(cx,          cy + s / 6, berryRadius);
    drawBerry(cx + s / 5,  cy + s / 8, berryRadius);
    drawBerry(cx - s / 10, cy + s / 3, berryRadius);
    drawBerry(cx + s / 10, cy + s / 3, berryRadius);
}
void Renderer::drawLeafEllipse(
    int cx,
    int cy,
    int rx,
    int ry,
    SDL_Color fillColor,
    SDL_Color veinColor
)
{
    /*
     * Draws a filled ellipse used as a stylized leaf.
     */

    if (rx <= 0 || ry <= 0) {
        return;
    }

    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(
        sdlRenderer,
        fillColor.r,
        fillColor.g,
        fillColor.b,
        fillColor.a
    );

    /*
     * Fill ellipse.
     */
    for (int y = -ry; y <= ry; y++) {
        for (int x = -rx; x <= rx; x++) {

            double nx = static_cast<double>(x) / static_cast<double>(rx);
            double ny = static_cast<double>(y) / static_cast<double>(ry);

            if (nx * nx + ny * ny <= 1.0) {
                SDL_RenderDrawPoint(sdlRenderer, cx + x, cy + y);
            }
        }
    }

    /*
     * Draw central vein.
     */
    SDL_SetRenderDrawColor(
        sdlRenderer,
        veinColor.r,
        veinColor.g,
        veinColor.b,
        veinColor.a
    );

    SDL_RenderDrawLine(
        sdlRenderer,
        cx,
        cy - ry,
        cx,
        cy + ry
    );

    /*
     * Draw side veins.
     */
    SDL_RenderDrawLine(
        sdlRenderer,
        cx,
        cy - ry / 2,
        cx - rx / 2,
        cy - ry / 4
    );

    SDL_RenderDrawLine(
        sdlRenderer,
        cx,
        cy - ry / 2,
        cx + rx / 2,
        cy - ry / 4
    );

    SDL_RenderDrawLine(
        sdlRenderer,
        cx,
        cy,
        cx - rx / 2,
        cy + ry / 6
    );

    SDL_RenderDrawLine(
        sdlRenderer,
        cx,
        cy,
        cx + rx / 2,
        cy + ry / 6
    );
}

void Renderer::drawBerryCluster(int cx, int cy, int berryRadius)
{
    /*
     * Draws a small cluster of berries.
     */

    if (berryRadius <= 0) {
        return;
    }

    SDL_Color berryRed       = { 220, 30, 40, 255 };
    SDL_Color berryDark      = { 150, 0, 25, 255 };
    SDL_Color berryHighlight = { 255, 150, 160, 255 };

    /*
     * Berry positions around the cluster center.
     */
    int offsets[5][2] = {
        { -berryRadius, 0 },
        { 0, 0 },
        { berryRadius, 0 },
        { -berryRadius / 2, berryRadius },
        { berryRadius / 2, berryRadius }
    };

    for (int i = 0; i < 5; i++) {
        int bx = cx + offsets[i][0];
        int by = cy + offsets[i][1];

        drawFilledCircle(bx, by, berryRadius, berryRed);

        drawFilledCircle(
            bx + berryRadius / 3,
            by + berryRadius / 3,
            std::max(1, berryRadius / 2),
                         berryDark
        );

        drawFilledCircle(
            bx - berryRadius / 3,
            by - berryRadius / 3,
            std::max(1, berryRadius / 3),
                         berryHighlight
        );
    }
}
void Renderer::drawBushModel(const SDL_Rect& cell, bool hasBerry)
{
    int s = cell.w < cell.h ? cell.w : cell.h;

    if (s < 6) {
        // Very small zoom fallback.
        SDL_Color bushColor = hasBerry
        ? SDL_Color{ 70, 160, 70, 255 }
        : SDL_Color{ 110, 190, 110, 255 };

        drawRect(cell, bushColor, true);

        if (hasBerry) {
            SDL_Rect berryRect;
            berryRect.x = cell.x + cell.w / 3;
            berryRect.y = cell.y + cell.h / 3;
            berryRect.w = cell.w / 3;
            berryRect.h = cell.h / 3;
            drawRect(berryRect, { 220, 30, 30, 255 }, true);
        }
        return;
    }

    int cx = cell.x + cell.w / 2;
    int cy = cell.y + cell.h / 2;

    SDL_Color outerLeaf = { 60, 140, 50, 255 };
    SDL_Color midLeaf   = { 90, 180, 70, 255 };
    SDL_Color innerLeaf = { 150, 220, 110, 255 };
    SDL_Color veinColor = { 50, 120, 45, 255 };

    int leafRxOuter = s / 9;
    int leafRyOuter = s / 6;

    int leafRxMid   = s / 10;
    int leafRyMid   = s / 6;

    int leafRxInner = s / 11;
    int leafRyInner = s / 7;

    // OUTER RING
    int outerOffsets[12][2] = {
        { -s/4, -s/4 }, { 0, -s/3 }, { s/4, -s/4 },
        { -s/3, 0 },    { s/3, 0 },
        { -s/4, s/4 },  { 0, s/3 },  { s/4, s/4 },
        { -s/5, -s/3 }, { s/5, -s/3 },
        { -s/5, s/3 },  { s/5, s/3 }
    };

    for (int i = 0; i < 12; i++) {
        drawLeafEllipse(
            cx + outerOffsets[i][0],
            cy + outerOffsets[i][1],
            leafRxOuter,
            leafRyOuter,
            outerLeaf,
            veinColor
        );
    }

    // MIDDLE RING
    int midOffsets[10][2] = {
        { -s/5, -s/5 }, { 0, -s/5 }, { s/5, -s/5 },
        { -s/5, 0 },    { s/5, 0 },
        { -s/5, s/5 },  { 0, s/5 },  { s/5, s/5 },
        { -s/7, -s/8 }, { s/7, -s/8 }
    };

    for (int i = 0; i < 10; i++) {
        drawLeafEllipse(
            cx + midOffsets[i][0],
            cy + midOffsets[i][1],
            leafRxMid,
            leafRyMid,
            midLeaf,
            veinColor
        );
    }

    // INNER CORE
    int innerOffsets[7][2] = {
        { 0, 0 },
        { -s/8, -s/8 }, { s/8, -s/8 },
        { -s/8, s/8 },  { s/8, s/8 },
        { 0, -s/6 },    { 0, s/6 }
    };

    for (int i = 0; i < 7; i++) {
        drawLeafEllipse(
            cx + innerOffsets[i][0],
            cy + innerOffsets[i][1],
            leafRxInner,
            leafRyInner,
            innerLeaf,
            veinColor
        );
    }

    // If the bush has berries, add 3 berry clusters like the reference.
    if (hasBerry) {
        int berryRadius = s / 12;
        if (berryRadius < 2) berryRadius = 2;

        drawBerryCluster(cx - s / 6, cy - s / 8, berryRadius);
        drawBerryCluster(cx + s / 6, cy - s / 8, berryRadius);
        drawBerryCluster(cx,         cy + s / 6, berryRadius);
    }
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

void Renderer::drawMap(MAP map, int MAP_W, int MAP_H, DISPLAY_OPTIONS& options)
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
						/*
						* Bushes use the full cell at low zoom.
						* Do not use resourceRect here.
						*/
						drawBushModel(cell, map[x][y].has_berry);
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