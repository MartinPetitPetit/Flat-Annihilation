#include "startmenu.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "../display/display.hpp"

int Display_start_menu(int argc, char* argv[])
{
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
        strcpy(font_path, "FreeSans.ttf");
    else if (argc == 2)
        strcpy(font_path, argv[1]);
    else {
        fprintf(stderr, "error: too many arguments\n");
        return EXIT_FAILURE;
    }

    TTF_Font *font = TTF_OpenFont(font_path, 24);
    if (!font) {
        fprintf(stderr, "error: font not found: %s\n", TTF_GetError());
        TTF_Quit(); SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Window*   pWindow  { nullptr };
    SDL_Renderer* pRenderer{ nullptr };

    if (SDL_CreateWindowAndRenderer(WIDTHSCREEN<int>, HEIGHTSCREEN<int>,
        SDL_WINDOW_SHOWN, &pWindow, &pRenderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
        TTF_CloseFont(font); TTF_Quit(); SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Event events;
    bool isOpen{ true };

    const char  *tooltip = "Tooltip example";
    SDL_Texture *tooltipTexture = nullptr;
    SDL_Rect     tooltipRect;
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

            case SDL_MOUSEBUTTONDOWN:
                if (events.button.button == SDL_BUTTON_LEFT) {
                    // TODO: gestion du clic
                }
                break;  // ← était en dehors du switch, donc ignoré
            }
        }

        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        // Rendu du texte DANS la loop, AVANT RenderPresent
        SDL_RenderCopy(pRenderer, tooltipTexture, NULL, &tooltipRect);

        SDL_RenderPresent(pRenderer); // ← manquait complètement
    }

    // Libération APRÈS la loop
    SDL_DestroyTexture(tooltipTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();

    return EXIT_SUCCESS;
}