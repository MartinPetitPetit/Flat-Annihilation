/*
 * Frontend/Renderer/Renderer.cpp
 *
 * Rôle du fichier :
 * Wraps SDL rendering operations, font loading, map drawing, zooming, viewport management, and simple drawing helpers.
 *
 * Notes de lecture :
 * Ce module centralise le rendu SDL afin que le reste du frontend n'appelle pas directement toutes les primitives bas niveau.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#include "Renderer.hpp"
#include "../../Backend/ResourceManager/ResourceManager.hpp"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

/*
 * Fonctions internes au fichier : elles servent surtout à trouver une police
 * même si le jeu est lancé depuis un autre dossier de travail.
 */
namespace
{
    void addCandidate(std::vector<std::string>& candidates, const std::string& path)
    {
        if (path.empty()) {
            return;
        }

        for (const std::string& candidate : candidates) {
            if (candidate == path) {
                return;
            }
        }

        candidates.push_back(path);
    }

    TTF_Font* openFontWithFallbacks(const char* fontPath, int size)
    {
        std::vector<std::string> candidates;

        if (fontPath != nullptr) {
            std::string basePath(fontPath);
            addCandidate(candidates, basePath);
            addCandidate(candidates, "src/" + basePath);
            addCandidate(candidates, "../" + basePath);
            addCandidate(candidates, "../../" + basePath);
        }

        addCandidate(candidates, "assets/fonts/Starjedi.ttf");
        addCandidate(candidates, "assets/Starjedi.ttf");
        addCandidate(candidates, "Starjedi.ttf");
        addCandidate(candidates, "src/assets/fonts/Starjedi.ttf");
        addCandidate(candidates, "src/assets/Starjedi.ttf");
        addCandidate(candidates, "src/Starjedi.ttf");
        addCandidate(candidates, "../assets/fonts/Starjedi.ttf");
        addCandidate(candidates, "../assets/Starjedi.ttf");
        addCandidate(candidates, "../../assets/fonts/Starjedi.ttf");
        addCandidate(candidates, "../../assets/Starjedi.ttf");

        for (const std::string& candidate : candidates) {
            TTF_Font* loadedFont = TTF_OpenFont(candidate.c_str(), size);

            if (loadedFont != nullptr) {
                return loadedFont;
            }
        }

        std::cerr << "Renderer: impossible de charger la police. Chemins testés:";
        for (const std::string& candidate : candidates) {
            std::cerr << " " << candidate;
        }
        std::cerr << "\n";

        return nullptr;
    }
}

/*
 * Crée le renderer SDL, l'enregistre dans le ResourceManager et charge la police.
 */
Renderer::Renderer(Window& window, const char* font_path)
{
    sdlRenderer = SDL_CreateRenderer(
        window.getSDLWindow(), -1,
                                     SDL_RENDERER_ACCELERATED
    );
    ResourceManager::getInstance().setRenderer(sdlRenderer);

    font = openFontWithFallbacks(font_path, 18);
}


/*
 * Libère les ressources SDL détenues par le renderer.
 */
Renderer::~Renderer()
{
    if (font)        TTF_CloseFont(font);
    if (sdlRenderer) SDL_DestroyRenderer(sdlRenderer);
}

/*
 * Efface l'image courante avant de dessiner une nouvelle frame.
 */
void Renderer::clear()
{
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
}

/*
 * Affiche à l'écran tout ce qui a été dessiné pendant la frame.
 */
void Renderer::present()
{
    SDL_RenderPresent(sdlRenderer);
}

/*
 * Copie une texture SDL vers la cible de rendu.
 */
void Renderer::drawTexture(SDL_Texture* tex, SDL_Rect* src, SDL_Rect* dst)
{
    SDL_RenderCopy(sdlRenderer, tex, src, dst);
}

/*
 * Dessine un rectangle rempli ou seulement sa bordure.
 */
void Renderer::drawRect(const SDL_Rect& rect, SDL_Color color, bool filled)
{
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
    if (filled) SDL_RenderFillRect(sdlRenderer, &rect);
    else        SDL_RenderDrawRect(sdlRenderer, &rect);
}

/*
 * Dessine un disque simple ligne par ligne.
 */
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

/*
 * Dessine un texte blanc avec la police chargée.
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

/*
 * Dessine la carte avec un niveau de détail adapté au zoom.
 * À faible zoom, plusieurs cellules sont regroupées pour améliorer les performances.
 */
void Renderer::drawMap(const MAP& map, int MAP_W, int MAP_H, DISPLAY_OPTIONS& options)
{
    int realWidth = static_cast<int>(map.size());
    if (realWidth == 0) return;
    int realHeight = static_cast<int>(map[0].size());
    if (realHeight == 0) return;

    if (MAP_W > realWidth)  MAP_W = realWidth;
    if (MAP_H > realHeight) MAP_H = realHeight;

    // -------------------------------------------------------
    // LOD : choisit la taille du bloc selon le zoom
    // -------------------------------------------------------
    int blockSize = 1;
    if (scale <= 1) blockSize = 4;
    else if (scale <= 3) blockSize = 2;

    bool drawResources = (scale >= 4);
    bool drawGrid      = (scale >= 8);

    // -------------------------------------------------------
    // Helper : couleur de terrain
    // -------------------------------------------------------
    auto terrainColor = [](TERRAIN t, int s) -> SDL_Color {
        switch (t) {
            case Plain:   return { 34, 139, 34, 255 };
            case Montain: return { 139, 69, 19, 255 };
            case Lake:    return { 0,   0, 180, 255 };
            case River:   return { 0, 120, 255, 255 };
            case Bush:    return { 0, 180,   0, 255 };
            case ravine:
                if (s <= 4) return {  0,  0,  0, 255 };
                if (s <= 8) return { 25, 25, 25, 255 };
                return              { 50, 50, 50, 255 };
            default:      return {   0,  0,  0, 255 };
        }
    };

    // -------------------------------------------------------
    // Rendu par blocs
    // -------------------------------------------------------
    for (int x = 0; x < MAP_W; x += blockSize) {
        for (int y = 0; y < MAP_H; y += blockSize) {

            // --- Calcul du rect écran du bloc ---
            SDL_Rect cell;
            cell.x = offsetX + x * scale;
            cell.y = offsetY + y * scale;
            cell.w = scale * blockSize;
            cell.h = scale * blockSize;

            // Culling
            if (cell.x + cell.w < 0 || cell.x > options.width)  continue;
            if (cell.y + cell.h < 0 || cell.y > options.height) continue;

            // -----------------------------------------------
            // LOD élevé (scale >= 4) : une cellule = un rect
            // -----------------------------------------------
            if (blockSize == 1) {
                SDL_Color color = terrainColor(map[x][y].type_terrain, scale);
                drawRect(cell, color, true);

                // Ressources
                if (drawResources && map[x][y].resource != nullptr) {
                    SDL_Rect resourceRect = {
                        cell.x + cell.w / 4,
                        cell.y + cell.h / 4,
                        std::max(1, cell.w / 2),
                        std::max(1, cell.h / 2)
                    };
                    switch (map[x][y].resource->getResourceType()) {
                        case wood:
                            map[x][y].resource->render(sdlRenderer, resourceRect);
                            break;
                        case stone:
                            drawRect(resourceRect, { 120, 120, 120, 255 }, true);
                            break;
                        case gold:
                            drawRect(resourceRect, { 255, 215,   0, 255 }, true);
                            break;
                        case food:
                            map[x][y].resource->render(sdlRenderer, resourceRect);
                            break;
                        default: break;
                    }
                }

                // Grille
                if (drawGrid)
                    drawRect(cell, { 0, 0, 0, 40 }, false);
            }

            // -----------------------------------------------
            // LOD bas (blocs 2x2 ou 4x4) : couleur dominante
            // -----------------------------------------------
            else {
                // On compte les terrains dans le bloc et on prend le dominant
                int counts[6] = {0,0,0,0,0,0};
                int bx_max = std::min(x + blockSize, MAP_W);
                int by_max = std::min(y + blockSize, MAP_H);

                for (int bx = x; bx < bx_max; bx++)
                    for (int by = y; by < by_max; by++)
                        counts[(int)map[bx][by].type_terrain]++;

                TERRAIN dominant = Plain;
                int     best     = 0;
                for (int t = 0; t < 6; t++) {
                    if (counts[t] > best) {
                        best     = counts[t];
                        dominant = static_cast<TERRAIN>(t);
                    }
                }

                SDL_Color color = terrainColor(dominant, scale);
                drawRect(cell, color, true);
            }
        }
    }
}
/*
 * Modifie le zoom tout en gardant la cellule sous la souris au même endroit visuel.
 */
void Renderer::applyZoom(int mouseX, int mouseY, int direction)
{
    int oldScale = scale;
    if (direction > 0 && scale < 64) scale++;
    else if (direction < 0 && scale > 2) scale--;
    offsetX = mouseX - (mouseX - offsetX) * scale / oldScale;
    offsetY = mouseY - (mouseY - offsetY) * scale / oldScale;
}
/*
 * Met à jour la zone de rendu après un changement de résolution.
 */
void Renderer::updateViewport(int w, int h) {
    SDL_RenderSetLogicalSize(sdlRenderer, 0, 0); // désactive le scaling automatique
    SDL_Rect vp = { 0, 0, w, h };
    SDL_RenderSetViewport(sdlRenderer, &vp);
}
/*
 * Change le décalage caméra utilisé pour convertir carte -> écran.
 */
void Renderer::setOffset(int x, int y) { offsetX = x; offsetY = y; }
int  Renderer::getOffsetX() const      { return offsetX; }
int  Renderer::getOffsetY() const      { return offsetY; }
int  Renderer::getScale()   const      { return scale;   }
bool Renderer::isValid()    const      { return sdlRenderer != nullptr; }
SDL_Renderer* Renderer::getSDLRenderer() const { return sdlRenderer; }
TTF_Font*     Renderer::getFont()        const { return font; }
