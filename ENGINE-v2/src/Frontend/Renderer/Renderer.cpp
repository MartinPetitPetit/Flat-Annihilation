#include "Renderer.hpp"

#include <algorithm>
#include <cmath>

/*
 * ============================================================
 * CONSTRUCTOR / DESTRUCTOR
 * ============================================================
 */

Renderer::Renderer(Window& window, const char* font_path)
{
    sdlRenderer = SDL_CreateRenderer(
        window.getSDLWindow(),
                                     -1,
                                     SDL_RENDERER_ACCELERATED
    );

    font = TTF_OpenFont(font_path, 18);
}

Renderer::~Renderer()
{
    if (font) {
        TTF_CloseFont(font);
    }

    if (sdlRenderer) {
        SDL_DestroyRenderer(sdlRenderer);
    }
}

/*
 * ============================================================
 * BASIC DRAWING FUNCTIONS
 * ============================================================
 */

unsigned int Renderer::hashCell(int x, int y) const
{
    unsigned int h = static_cast<unsigned int>(x * 73856093u);
    h ^= static_cast<unsigned int>(y * 19349663u);
    return h;
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

    if (filled) {
        SDL_RenderFillRect(sdlRenderer, &rect);
    }
    else {
        SDL_RenderDrawRect(sdlRenderer, &rect);
    }
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
 * TREE DRAWING
 * ============================================================
 */

void Renderer::drawTreeModel(const SDL_Rect& cellRect, WOOD_TYPE wood_type)
{
    switch (wood_type) {
        case Wood_A:
            drawRoundTree(cellRect);
            break;

        case Wood_B:
            drawPineTree(cellRect);
            break;

        case Wood_C:
            drawSmallTree(cellRect);
            break;

        case No_Wood:
        default:
            drawRoundTree(cellRect);
            break;
    }
}

void Renderer::drawRoundTree(const SDL_Rect& cellRect)
{
    if (cellRect.w < 6 || cellRect.h < 6) {
        SDL_Rect smallTree;
        smallTree.x = cellRect.x + cellRect.w / 4;
        smallTree.y = cellRect.y + cellRect.h / 4;
        smallTree.w = std::max(1, cellRect.w / 2);
        smallTree.h = std::max(1, cellRect.h / 2);

        drawRect(smallTree, { 0, 80, 0, 255 }, true);
        return;
    }

    int cx = cellRect.x + cellRect.w / 2;
    int cy = cellRect.y + cellRect.h / 2;

    int trunkW = std::max(1, cellRect.w / 5);
    int trunkH = std::max(2, cellRect.h / 3);

    SDL_Rect trunk;
    trunk.x = cx - trunkW / 2;
    trunk.y = cellRect.y + cellRect.h / 2;
    trunk.w = trunkW;
    trunk.h = trunkH;

    drawRect(trunk, { 100, 60, 20, 255 }, true);

    int r = std::max(2, cellRect.w / 4);

    drawFilledCircle(cx, cy - cellRect.h / 8, r, { 0, 100, 0, 255 });
    drawFilledCircle(cx - r / 2, cy, r, { 0, 130, 0, 255 });
    drawFilledCircle(cx + r / 2, cy, r, { 0, 90, 0, 255 });
}

void Renderer::drawPineTree(const SDL_Rect& cellRect)
{
    if (cellRect.w < 6 || cellRect.h < 6) {
        SDL_Rect smallTree;
        smallTree.x = cellRect.x + cellRect.w / 4;
        smallTree.y = cellRect.y + cellRect.h / 4;
        smallTree.w = std::max(1, cellRect.w / 2);
        smallTree.h = std::max(1, cellRect.h / 2);

        drawRect(smallTree, { 0, 110, 0, 255 }, true);
        return;
    }

    int cx = cellRect.x + cellRect.w / 2;

    int trunkW = std::max(1, cellRect.w / 6);
    int trunkH = std::max(2, cellRect.h / 4);

    SDL_Rect trunk;
    trunk.x = cx - trunkW / 2;
    trunk.y = cellRect.y + (cellRect.h * 2) / 3;
    trunk.w = trunkW;
    trunk.h = trunkH;

    drawRect(trunk, { 90, 55, 20, 255 }, true);

    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

    int topY = cellRect.y + cellRect.h / 8;
    int midY = cellRect.y + cellRect.h / 2;
    int bottomY = cellRect.y + (cellRect.h * 3) / 4;

    SDL_SetRenderDrawColor(sdlRenderer, 0, 90, 0, 255);

    for (int y = topY; y <= midY; y++) {
        int relative = y - topY;
        int halfWidth = (relative * cellRect.w) / cellRect.h;

        SDL_RenderDrawLine(
            sdlRenderer,
            cx - halfWidth,
            y,
            cx + halfWidth,
            y
        );
    }

    SDL_SetRenderDrawColor(sdlRenderer, 0, 120, 0, 255);

    for (int y = cellRect.y + cellRect.h / 3; y <= bottomY; y++) {
        int relative = y - (cellRect.y + cellRect.h / 3);
        int halfWidth = (relative * cellRect.w) / cellRect.h + cellRect.w / 8;

        if (halfWidth > cellRect.w / 2) {
            halfWidth = cellRect.w / 2;
        }

        SDL_RenderDrawLine(
            sdlRenderer,
            cx - halfWidth,
            y,
            cx + halfWidth,
            y
        );
    }
}

void Renderer::drawSmallTree(const SDL_Rect& cellRect)
{
    if (cellRect.w < 6 || cellRect.h < 6) {
        SDL_Rect smallTree;
        smallTree.x = cellRect.x + cellRect.w / 4;
        smallTree.y = cellRect.y + cellRect.h / 4;
        smallTree.w = std::max(1, cellRect.w / 2);
        smallTree.h = std::max(1, cellRect.h / 2);

        drawRect(smallTree, { 60, 130, 40, 255 }, true);
        return;
    }

    int cx = cellRect.x + cellRect.w / 2;
    int cy = cellRect.y + cellRect.h / 2;

    int trunkW = std::max(1, cellRect.w / 6);
    int trunkH = std::max(2, cellRect.h / 3);

    SDL_Rect trunk;
    trunk.x = cx - trunkW / 2;
    trunk.y = cy;
    trunk.w = trunkW;
    trunk.h = trunkH;

    drawRect(trunk, { 130, 85, 35, 255 }, true);

    int r = std::max(2, cellRect.w / 5);

    drawFilledCircle(cx, cy - r, r, { 80, 150, 40, 255 });
    drawFilledCircle(cx - r, cy, r, { 60, 130, 40, 255 });
    drawFilledCircle(cx + r, cy, r, { 90, 160, 50, 255 });
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

void Renderer::drawBerryBushModel(const SDL_Rect& cellRect, bool hasBerry)
{
    int cx = cellRect.x + cellRect.w / 2;
    int cy = cellRect.y + cellRect.h / 2;

    int s = cellRect.w;

    if (cellRect.h < s) {
        s = cellRect.h;
    }

    if (s < 4) {
        return;
    }

    int leafSize = std::max(2, s / 5);
    int berryRadius = std::max(2, s / 8);

    SDL_SetRenderDrawColor(sdlRenderer, 80, 50, 25, 255);

    SDL_RenderDrawLine(sdlRenderer, cx, cy, cx - s / 4, cy - s / 5);
    SDL_RenderDrawLine(sdlRenderer, cx, cy, cx + s / 4, cy - s / 5);
    SDL_RenderDrawLine(sdlRenderer, cx, cy, cx - s / 6, cy + s / 4);
    SDL_RenderDrawLine(sdlRenderer, cx, cy, cx + s / 6, cy + s / 4);

    drawLeaf(cx - s / 4, cy - s / 4, leafSize, { 70, 160, 60, 255 });
    drawLeaf(cx + s / 4, cy - s / 4, leafSize, { 90, 180, 65, 255 });
    drawLeaf(cx,       cy - s / 3, leafSize, { 60, 140, 55, 255 });

    if (!hasBerry) {
        drawLeaf(cx - s / 8, cy + s / 8, leafSize, { 50, 130, 45, 255 });
        drawLeaf(cx + s / 8, cy + s / 8, leafSize, { 70, 150, 50, 255 });
        return;
    }

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

    for (int y = -ry; y <= ry; y++) {
        for (int x = -rx; x <= rx; x++) {

            double nx = static_cast<double>(x) / static_cast<double>(rx);
            double ny = static_cast<double>(y) / static_cast<double>(ry);

            if (nx * nx + ny * ny <= 1.0) {
                SDL_RenderDrawPoint(sdlRenderer, cx + x, cy + y);
            }
        }
    }

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
    if (berryRadius <= 0) {
        return;
    }

    SDL_Color berryRed       = { 220, 30, 40, 255 };
    SDL_Color berryDark      = { 150, 0, 25, 255 };
    SDL_Color berryHighlight = { 255, 150, 160, 255 };

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

void Renderer::drawBushModel(const SDL_Rect& cellRect, bool hasBerry)
{
    int s = cellRect.w < cellRect.h ? cellRect.w : cellRect.h;

    if (s < 6) {
        SDL_Color bushColor = hasBerry
        ? SDL_Color{ 70, 160, 70, 255 }
        : SDL_Color{ 110, 190, 110, 255 };

        drawRect(cellRect, bushColor, true);

        if (hasBerry) {
            SDL_Rect berryRect;
            berryRect.x = cellRect.x + cellRect.w / 3;
            berryRect.y = cellRect.y + cellRect.h / 3;
            berryRect.w = std::max(1, cellRect.w / 3);
            berryRect.h = std::max(1, cellRect.h / 3);

            drawRect(berryRect, { 220, 30, 30, 255 }, true);
        }

        return;
    }

    int cx = cellRect.x + cellRect.w / 2;
    int cy = cellRect.y + cellRect.h / 2;

    SDL_Color outerLeaf = { 60, 140, 50, 255 };
    SDL_Color midLeaf   = { 90, 180, 70, 255 };
    SDL_Color innerLeaf = { 150, 220, 110, 255 };
    SDL_Color veinColor = { 50, 120, 45, 255 };

    int leafRxOuter = std::max(1, s / 9);
    int leafRyOuter = std::max(1, s / 6);

    int leafRxMid   = std::max(1, s / 10);
    int leafRyMid   = std::max(1, s / 6);

    int leafRxInner = std::max(1, s / 11);
    int leafRyInner = std::max(1, s / 7);

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

    if (hasBerry) {
        int berryRadius = s / 12;
        if (berryRadius < 2) {
            berryRadius = 2;
        }

        drawBerryCluster(cx - s / 6, cy - s / 8, berryRadius);
        drawBerryCluster(cx + s / 6, cy - s / 8, berryRadius);
        drawBerryCluster(cx,         cy + s / 6, berryRadius);
    }
}

/*
 * ============================================================
 * STRUCTURE / UNIT DRAWING
 * ============================================================
 */

void Renderer::drawBaseModel(const SDL_Rect& cellRect)
{
    SDL_Rect base;

    base.w = std::max(1, cellRect.w * 3 / 4);
    base.h = std::max(1, cellRect.h * 3 / 4);

    base.x = cellRect.x + cellRect.w / 8;
    base.y = cellRect.y + cellRect.h / 8;

    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
    SDL_RenderFillRect(sdlRenderer, &base);

    SDL_SetRenderDrawColor(sdlRenderer, 40, 40, 40, 255);
    SDL_RenderDrawRect(sdlRenderer, &base);

    if (cellRect.w >= 10 && cellRect.h >= 10) {
        SDL_Rect core;
        core.w = std::max(1, cellRect.w / 3);
        core.h = std::max(1, cellRect.h / 3);
        core.x = cellRect.x + cellRect.w / 3;
        core.y = cellRect.y + cellRect.h / 3;

        SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
        SDL_RenderFillRect(sdlRenderer, &core);
    }
}

void Renderer::drawCollectorModel(const SDL_Rect& cellRect)
{
    SDL_Rect body;

    body.w = std::max(1, cellRect.w / 2);
    body.h = std::max(1, cellRect.h / 2);

    body.x = cellRect.x + cellRect.w / 4;
    body.y = cellRect.y + cellRect.h / 4;

    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(sdlRenderer, 220, 220, 40, 255);
    SDL_RenderFillRect(sdlRenderer, &body);

    SDL_SetRenderDrawColor(sdlRenderer, 80, 80, 20, 255);
    SDL_RenderDrawRect(sdlRenderer, &body);

    if (cellRect.w >= 10 && cellRect.h >= 10) {
        int cx = cellRect.x + cellRect.w / 2;
        int cy = cellRect.y + cellRect.h / 2;

        drawFilledCircle(cx, cy, std::max(1, cellRect.w / 8), { 255, 255, 120, 255 });
    }
}

/*
 * ============================================================
 * TEXT DRAWING
 * ============================================================
 */

void Renderer::drawText(const char* text, int x, int y)
{
    if (!font) {
        return;
    }

    SDL_Color white = { 255, 255, 255, 255 };

    SDL_Surface* surf = TTF_RenderText_Solid(font, text, white);

    if (!surf) {
        return;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(sdlRenderer, surf);

    if (!tex) {
        SDL_FreeSurface(surf);
        return;
    }

    SDL_Rect dst = {
        x,
        y,
        surf->w,
        surf->h
    };

    SDL_RenderCopy(sdlRenderer, tex, NULL, &dst);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

/*
 * ============================================================
 * MAP DRAWING
 * ============================================================
 */

void Renderer::drawMap(MAP& map, int MAP_W, int MAP_H, DISPLAY_OPTIONS& options)
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

            SDL_Rect cellRect;
            cellRect.x = offsetX + x * scale;
            cellRect.y = offsetY + y * scale;
            cellRect.w = scale;
            cellRect.h = scale;

            if (cellRect.x + cellRect.w < 0 || cellRect.x > options.width) {
                continue;
            }

            if (cellRect.y + cellRect.h < 0 || cellRect.y > options.height) {
                continue;
            }

            Cell& mapCell = map[x][y];

            /*
             * ==================================================
             * PART 1: TERRAIN
             * ==================================================
             */

            SDL_Color color;

            switch (mapCell.type_terrain) {
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

            drawRect(cellRect, color, true);

            /*
             * ==================================================
             * PART 2: RESOURCE OVER TERRAIN
             * ==================================================
             */

            SDL_Rect resourceRect;
            resourceRect.x = cellRect.x + cellRect.w / 4;
            resourceRect.y = cellRect.y + cellRect.h / 4;
            resourceRect.w = std::max(1, cellRect.w / 2);
            resourceRect.h = std::max(1, cellRect.h / 2);

            switch (mapCell.type_resource) {
                case tree:
                    drawTreeModel(cellRect, mapCell.wood_type);
                    break;

                case stone:
                    drawRect(resourceRect, { 120, 120, 120, 255 }, true);
                    break;

                case gold:
                    drawRect(resourceRect, { 255, 215, 0, 255 }, true);
                    break;

                case iron:
                    drawRect(resourceRect, { 90, 90, 90, 255 }, true);
                    break;

                case Sapling:
                    drawRect(resourceRect, { 0, 180, 60, 255 }, true);
                    break;

                case BushResource:
                    drawBushModel(cellRect, mapCell.has_berry);
                    break;

                case None_Resource:
                default:
                    break;
            }

            /*
             * ==================================================
             * PART 3: STRUCTURE OVER RESOURCE
             * ==================================================
             */

            switch (mapCell.type_struct) {
                case Base:
                    drawBaseModel(cellRect);
                    break;

                case Usine:
                    /*
                     * drawUsineModel(cellRect);
                     * Not implemented yet.
                     */
                    break;

                case Production:
                    /*
                     * drawProductionModel(cellRect);
                     * Not implemented yet.
                     */
                    break;

                default:
                    break;
            }

            /*
             * ==================================================
             * PART 4: UNIT OVER STRUCTURE
             * ==================================================
             */

            switch (mapCell.type_unit) {
                case Collector_Unit:
                    drawCollectorModel(cellRect);
                    break;

                case Archer_Unit:
                    /*
                     * drawArcherModel(cellRect);
                     * Not implemented yet.
                     */
                    break;

                case Monk_Unit:
                    /*
                     * drawMonkModel(cellRect);
                     * Not implemented yet.
                     */
                    break;

                default:
                    break;
            }

            /*
             * ==================================================
             * PART 5: OPTIONAL GRID
             * ==================================================
             */

            if (scale >= 8) {
                drawRect(cellRect, { 0, 0, 0, 40 }, false);
            }
        }
    }
}

/*
 * ============================================================
 * CAMERA / ZOOM
 * ============================================================
 */

void Renderer::applyZoom(int mouseX, int mouseY, int direction)
{
    int oldScale = scale;

    if (direction > 0 && scale < 64) {
        scale++;
    }
    else if (direction < 0 && scale > 2) {
        scale--;
    }

    offsetX = mouseX - (mouseX - offsetX) * scale / oldScale;
    offsetY = mouseY - (mouseY - offsetY) * scale / oldScale;
}

void Renderer::setOffset(int x, int y)
{
    offsetX = x;
    offsetY = y;
}

int Renderer::getOffsetX() const
{
    return offsetX;
}

int Renderer::getOffsetY() const
{
    return offsetY;
}

int Renderer::getScale() const
{
    return scale;
}

bool Renderer::isValid() const
{
    return sdlRenderer != nullptr;
}

SDL_Renderer* Renderer::getSDLRenderer() const
{
    return sdlRenderer;
}

TTF_Font* Renderer::getFont() const
{
    return font;
}
