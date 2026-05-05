#include "display.hpp"
#include "../startmenu/startmenu.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>


void draw_case_fill(SDL_Renderer* renderer, const SDL_Rect& rectangle, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rectangle);
}

void draw_rectangle_not_fill(SDL_Renderer* renderer, const SDL_Rect& rectangle, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rectangle);
}

void get_text_and_rect(SDL_Renderer *renderer, int x, int y, const char *text,
        TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect) {
    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, textColor);
    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect->x = x;
    rect->y = y;
    rect->w = surface->w;
    rect->h = surface->h;
    SDL_FreeSurface(surface);
}

int DisplayMap(MAP& map, int argc, char* argv[],int MAP_W, int MAP_H,DISPLAY_OPTIONS& options) {
    

    // Création puis génération — propre et sans self-reference

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (TTF_Init() < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init: %s", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    char font_path[256];
    if (argc == 1)
        strcpy(font_path, "Starjedi.ttf");
    else if (argc == 2)
        strcpy(font_path, argv[1]);
    else {
        fprintf(stderr, "error: too many arguments\n");
        return EXIT_FAILURE;
    }

    TTF_Font *font = TTF_OpenFont(font_path, 18);

    if (!font) {
        fprintf(stderr, "error: font not found: %s\n", TTF_GetError());
        TTF_Quit(); SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Window*   pWindow  { nullptr };
    SDL_Renderer* pRenderer{ nullptr };

    if (SDL_CreateWindowAndRenderer(options.width, options.height,
        SDL_WINDOW_SHOWN, &pWindow, &pRenderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
        TTF_CloseFont(font); TTF_Quit(); SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Event events;
    bool isOpen        { true  };
    bool isBeingDragged{ false };

    int scale          = 8;
    int offsetX        = 0, offsetY        = 0;
    int dragStartX     = 0, dragStartY     = 0;
    int dragStartOffsetX = 0, dragStartOffsetY = 0;
    int selectedCellX  = -1, selectedCellY  = -1;

    const char   *tooltip = "il y a bien longtemps, dans une galaxie lointaine, tres lointaine...";
    SDL_Texture  *tooltipTexture = nullptr;
    SDL_Rect      tooltipRect;
    get_text_and_rect(pRenderer, 0, 0, tooltip, font, &tooltipTexture, &tooltipRect);

    while (isOpen)
    {
        while (SDL_PollEvent(&events))
        {
            switch (events.type)
            {
            case SDL_QUIT:
                isOpen = false;
                break;

            case SDL_MOUSEWHEEL: {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                int oldScale = scale;
                if (events.wheel.y > 0 && scale < 64) scale++;
                else if (events.wheel.y < 0 && scale > 2) scale--;
                offsetX = mouseX - (mouseX - offsetX) * scale / oldScale;
                offsetY = mouseY - (mouseY - offsetY) * scale / oldScale;
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
                if (events.button.button == SDL_BUTTON_RIGHT) {
                    isBeingDragged   = true;
                    dragStartX       = events.button.x;
                    dragStartY       = events.button.y;
                    dragStartOffsetX = offsetX;
                    dragStartOffsetY = offsetY;
                } else if (events.button.button == SDL_BUTTON_LEFT) {
                    int cx = (events.button.x - offsetX) / scale;
                    int cy = (events.button.y - offsetY) / scale;
                    if (cx >= 0 && cx < MAP_W && cy >= 0 && cy < MAP_H) {
                        selectedCellX = cx;
                        selectedCellY = cy;
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (events.button.button == SDL_BUTTON_RIGHT)
                    isBeingDragged = false;
                break;

            case SDL_MOUSEMOTION:
                if (isBeingDragged) {
                    offsetX = dragStartOffsetX + (events.motion.x - dragStartX);
                    offsetY = dragStartOffsetY + (events.motion.y - dragStartY);
                }
                break;
            }
        }

        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        SDL_Color color{ 255, 255, 255, 255 };

        for (int x = 0; x < MAP_W; x++) {
            for (int y = 0; y < MAP_H; y++) {
                SDL_Rect cell;
                cell.x = offsetX + x * scale;
                cell.y = offsetY + y * scale;
                cell.w = scale;
                cell.h = scale;

                if (cell.x + cell.w < 0 || cell.x > options.width) continue;
                if (cell.y + cell.h < 0 || cell.y > options.height) continue;

                switch (map[x][y].type_terrain) {
                    case Plain:   color = {   0, 255,   0, 255 }; break; // vert
                    case Montain: color = { 139,  69,  19, 255 }; break; // marron
                    case Lake:    color = {   0,   0, 255, 255 }; break; // bleu
                    case Bush:    color = {   0, 180,   0, 255 }; break; // vert foncé
                    case ravine:  color = { 100, 100, 100, 255 }; break; // gris
                }
                draw_case_fill(pRenderer, cell, color);
            }
        }

        if (selectedCellX >= 0 && selectedCellY >= 0) {
            SDL_Rect sel;
            sel.x = offsetX + selectedCellX * scale;
            sel.y = offsetY + selectedCellY * scale;
            sel.w = scale;
            sel.h = scale;
            draw_case_fill(pRenderer, sel, { 255, 0, 0, 255 });
        }

        SDL_RenderCopy(pRenderer, tooltipTexture, NULL, &tooltipRect);
        SDL_RenderPresent(pRenderer);
    }

    SDL_DestroyTexture(tooltipTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
    return EXIT_SUCCESS;
}