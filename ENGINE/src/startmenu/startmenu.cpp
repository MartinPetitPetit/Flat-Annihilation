#include "startmenu.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "../display/display.hpp"

typedef enum {
    MENU_GENERATE_MAP  = 0,
    MENU_SOLO_VS_IA    = 1,
    MENU_MULTIPLAYER   = 2,
    MENU_OPTIONS       = 3,
    MENU_QUIT          = -1
} MENU_RESULT;

int Display_options_menu(int argc, char* argv[], DISPLAY_OPTIONS& options)
{
    char font_path[256];
    if (argc == 1) strcpy(font_path, "Starjedi.ttf");
    else           strcpy(font_path, argv[1]);

    TTF_Font *font = TTF_OpenFont(font_path, 32);
    if (!font) return -1;

    SDL_Window*   pWindow  { nullptr };
    SDL_Renderer* pRenderer{ nullptr };

    if (SDL_CreateWindowAndRenderer(500, 300,
        SDL_WINDOW_SHOWN, &pWindow, &pRenderer) < 0) {
        TTF_CloseFont(font);
        return -1;
    }

    SDL_SetWindowPosition(pWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetWindowTitle(pWindow, "options");

    const int RES_COUNT = 4;
    const char *res_labels[RES_COUNT] = { "800x600", "1280x720", "1920x1080", "2560x1440" };
    const int res_w[RES_COUNT] = { 800, 1280, 1920, 2560 };
    const int res_h[RES_COUNT] = { 600,  720, 1080, 1440 };

    int currentRes = 0;
    for (int i = 0; i < RES_COUNT; i++)
        if (res_w[i] == options.width && res_h[i] == options.height)
            currentRes = i;

    bool fullscreen = options.fullscreen;

    const int ITEM_COUNT = 3;
    const char *item_labels[ITEM_COUNT] = { "resolution", "fullscreen", "back" };

    SDL_Color colorNormal   = { 200, 200, 200, 255 };
    SDL_Color colorSelected = { 255, 220,   0, 255 };
    SDL_Color colorValue    = {   0, 220, 255, 255 };

    int btnW = 460, btnH = 50;
    int startY = 300 / 2 - (ITEM_COUNT * (btnH + 15)) / 2;

    SDL_Rect rects[ITEM_COUNT];
    for (int i = 0; i < ITEM_COUNT; i++)
        rects[i] = { 500 / 2 - btnW / 2, startY + i * (btnH + 15), btnW, btnH };

    SDL_Texture *bgTexture = nullptr;
    SDL_Surface *bgSurface = IMG_Load("background.png");
    if (bgSurface) {
        bgTexture = SDL_CreateTextureFromSurface(pRenderer, bgSurface);
        SDL_FreeSurface(bgSurface);
    }

    SDL_Event events;
    bool isOpen        { true };
    int  selectedIndex { 0   };
    Uint32 windowID = SDL_GetWindowID(pWindow);

    while (isOpen)
    {
        while (SDL_PollEvent(&events))
        {
            // Fermeture de la fenêtre options
            if (events.type == SDL_WINDOWEVENT &&
                events.window.windowID == windowID &&
                events.window.event == SDL_WINDOWEVENT_CLOSE) {
                isOpen = false;
                break;
            }

            // Filtrer les events — ignorer ceux des autres fenêtres
            switch (events.type) {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    if (events.key.windowID != windowID) continue;
                    break;
                case SDL_MOUSEMOTION:
                    if (events.motion.windowID != windowID) continue;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    if (events.button.windowID != windowID) continue;
                    break;
                default:
                    break;
            }

            switch (events.type)
            {
            case SDL_KEYDOWN:
                switch (events.key.keysym.sym)
                {
                case SDLK_UP:
                    selectedIndex--;
                    if (selectedIndex < 0) selectedIndex = ITEM_COUNT - 1;
                    break;
                case SDLK_DOWN:
                    selectedIndex++;
                    if (selectedIndex >= ITEM_COUNT) selectedIndex = 0;
                    break;
                case SDLK_LEFT:
                    if (selectedIndex == 0) { currentRes--; if (currentRes < 0) currentRes = RES_COUNT - 1; }
                    if (selectedIndex == 1) fullscreen = !fullscreen;
                    break;
                case SDLK_RIGHT:
                    if (selectedIndex == 0) { currentRes++; if (currentRes >= RES_COUNT) currentRes = 0; }
                    if (selectedIndex == 1) fullscreen = !fullscreen;
                    break;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    if (selectedIndex == 2) isOpen = false;
                    break;
                case SDLK_ESCAPE:
                    isOpen = false;
                    break;
                }
                break;

            case SDL_MOUSEMOTION:
                for (int i = 0; i < ITEM_COUNT; i++) {
                    int mx = events.motion.x, my = events.motion.y;
                    if (mx >= rects[i].x && mx <= rects[i].x + rects[i].w &&
                        my >= rects[i].y && my <= rects[i].y + rects[i].h)
                        selectedIndex = i;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (events.button.button == SDL_BUTTON_LEFT) {
                    int mx = events.button.x, my = events.button.y;
                    for (int i = 0; i < ITEM_COUNT; i++) {
                        if (mx >= rects[i].x && mx <= rects[i].x + rects[i].w &&
                            my >= rects[i].y && my <= rects[i].y + rects[i].h) {
                            if      (i == 0) currentRes = (currentRes + 1) % RES_COUNT;
                            else if (i == 1) fullscreen = !fullscreen;
                            else if (i == 2) isOpen = false;
                        }
                    }
                }
                break;
            }
        }

        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        if (bgTexture)
            SDL_RenderCopy(pRenderer, bgTexture, NULL, NULL);

        for (int i = 0; i < ITEM_COUNT; i++) {
            SDL_SetRenderDrawBlendMode(pRenderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(pRenderer,
                i == selectedIndex ? 80 : 30,
                i == selectedIndex ? 80 : 30,
                i == selectedIndex ? 20 : 30, 180);
            SDL_RenderFillRect(pRenderer, &rects[i]);
            SDL_SetRenderDrawColor(pRenderer, 150, 150, 150, 255);
            SDL_RenderDrawRect(pRenderer, &rects[i]);

            SDL_Color col      = (i == selectedIndex) ? colorSelected : colorNormal;
            SDL_Surface *sSurf = TTF_RenderText_Blended(font, item_labels[i], col);
            SDL_Texture *sTex  = SDL_CreateTextureFromSurface(pRenderer, sSurf);
            int tw, th;
            SDL_QueryTexture(sTex, NULL, NULL, &tw, &th);
            SDL_Rect labelRect = { rects[i].x + 15, rects[i].y + (rects[i].h - th) / 2, tw, th };
            SDL_RenderCopy(pRenderer, sTex, NULL, &labelRect);
            SDL_FreeSurface(sSurf);
            SDL_DestroyTexture(sTex);

            char valueStr[64] = "";
            if      (i == 0) snprintf(valueStr, sizeof(valueStr), " %s ", res_labels[currentRes]);
            else if (i == 1) snprintf(valueStr, sizeof(valueStr), " %s ", fullscreen ? "on" : "off");

            if (valueStr[0] != '\0') {
                SDL_Surface *vSurf = TTF_RenderText_Blended(font, valueStr, colorValue);
                SDL_Texture *vTex  = SDL_CreateTextureFromSurface(pRenderer, vSurf);
                int vw, vh;
                SDL_QueryTexture(vTex, NULL, NULL, &vw, &vh);
                SDL_Rect valueRect = { rects[i].x + rects[i].w - vw - 15, rects[i].y + (rects[i].h - vh) / 2, vw, vh };
                SDL_RenderCopy(pRenderer, vTex, NULL, &valueRect);
                SDL_FreeSurface(vSurf);
                SDL_DestroyTexture(vTex);
            }
        }

        SDL_RenderPresent(pRenderer);
    }

    options.width      = res_w[currentRes];
    options.height     = res_h[currentRes];
    options.fullscreen = fullscreen;

    if (bgTexture) SDL_DestroyTexture(bgTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);

    return 0;
}

int Display_start_menu(int argc, char* argv[],DISPLAY_OPTIONS& options)
{
    // SDL init ici — sera nettoyé dans main
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
        return MENU_QUIT;
    }
    if (TTF_Init() < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init: %s", TTF_GetError());
        SDL_Quit();
        return MENU_QUIT;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "IMG_Init: %s", IMG_GetError());
        TTF_Quit(); SDL_Quit();
        return MENU_QUIT;
    }

    char font_path[256];
    if (argc == 1)      strcpy(font_path, "Starjedi.ttf");
    else if (argc == 2) strcpy(font_path, argv[1]);
    else {
        fprintf(stderr, "error: too many arguments\n");
        return MENU_QUIT;
    }

    TTF_Font *font = TTF_OpenFont(font_path, 32);
    if (!font) {
        fprintf(stderr, "error: font not found: %s\n", TTF_GetError());
        TTF_Quit(); SDL_Quit();
        return MENU_QUIT;
    }

    SDL_Window*   pWindow  { nullptr };
    SDL_Renderer* pRenderer{ nullptr };

    if (SDL_CreateWindowAndRenderer(options.width, options.height,
        SDL_WINDOW_SHOWN, &pWindow, &pRenderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
        TTF_CloseFont(font); TTF_Quit(); SDL_Quit();
        return MENU_QUIT;
    }

    SDL_Texture *bgTexture = nullptr;
    SDL_Surface *bgSurface = IMG_Load("background.png");
    if (bgSurface) {
        bgTexture = SDL_CreateTextureFromSurface(pRenderer, bgSurface);
        SDL_FreeSurface(bgSurface);
    }

    const int   BUTTON_COUNT = 4;
    const char *labels[BUTTON_COUNT] = {
        "generate map", "solo vs ia", "multiplayer", "options"
    };

    SDL_Texture *texNormal[BUTTON_COUNT]   = {};
    SDL_Texture *texSelected[BUTTON_COUNT] = {};
    SDL_Rect     rects[BUTTON_COUNT];

    SDL_Color colorNormal   = { 200, 200, 200, 255 };
    SDL_Color colorSelected = { 255, 220,   0, 255 };

    int btnW = 320, btnH = 50;
    int startY = options.height / 2 - (BUTTON_COUNT * (btnH + 15)) / 2;

    for (int i = 0; i < BUTTON_COUNT; i++) {
        rects[i] = { options.width / 2 - btnW / 2, startY + i * (btnH + 15), btnW, btnH };
        SDL_Surface *sNormal   = TTF_RenderText_Blended(font, labels[i], colorNormal);
        SDL_Surface *sSelected = TTF_RenderText_Blended(font, labels[i], colorSelected);
        texNormal[i]   = SDL_CreateTextureFromSurface(pRenderer, sNormal);
        texSelected[i] = SDL_CreateTextureFromSurface(pRenderer, sSelected);
        SDL_FreeSurface(sNormal);
        SDL_FreeSurface(sSelected);
    }

    SDL_Event events;
    bool isOpen        { true };
    int  selectedIndex { 0   };
    int  result        { MENU_QUIT };
    Uint32 windowID = SDL_GetWindowID(pWindow);

    while (isOpen)
    {
        while (SDL_PollEvent(&events))
        {
            // Filtrer les events de cette fenêtre
            switch (events.type) {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    if (events.key.windowID != windowID) continue;
                    break;
                case SDL_MOUSEMOTION:
                    if (events.motion.windowID != windowID) continue;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    if (events.button.windowID != windowID) continue;
                    break;
                default:
                    break;
            }

            switch (events.type)
            {
            case SDL_QUIT:
                isOpen = false;
                break;

            case SDL_KEYDOWN:
                switch (events.key.keysym.sym)
                {
                case SDLK_UP:
                    selectedIndex--;
                    if (selectedIndex < 0) selectedIndex = BUTTON_COUNT - 1;
                    break;
                case SDLK_DOWN:
                    selectedIndex++;
                    if (selectedIndex >= BUTTON_COUNT) selectedIndex = 0;
                    break;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    if (selectedIndex == MENU_OPTIONS) {
                        // Ouvrir options sans fermer le start menu
                        Display_options_menu(argc, argv, options);
                    } else {
                        result = selectedIndex;
                        isOpen = false;
                    }
                    break;
                case SDLK_ESCAPE:
                    isOpen = false;
                    break;
                }
                break;

            case SDL_MOUSEMOTION: {
                int mx = events.motion.x, my = events.motion.y;
                for (int i = 0; i < BUTTON_COUNT; i++)
                    if (mx >= rects[i].x && mx <= rects[i].x + rects[i].w &&
                        my >= rects[i].y && my <= rects[i].y + rects[i].h)
                        selectedIndex = i;
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
                if (events.button.button == SDL_BUTTON_LEFT) {
                    int mx = events.button.x, my = events.button.y;
                    for (int i = 0; i < BUTTON_COUNT; i++) {
                        if (mx >= rects[i].x && mx <= rects[i].x + rects[i].w &&
                            my >= rects[i].y && my <= rects[i].y + rects[i].h) {
                            if (i == MENU_OPTIONS) {
                                // Ouvrir options sans fermer le start menu
                                Display_options_menu(argc, argv, options);
                            } else {
                                result = i;
                                isOpen = false;
                            }
                        }
                    }
                }
                break;
            }
        }

        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        if (bgTexture)
            SDL_RenderCopy(pRenderer, bgTexture, NULL, NULL);

        for (int i = 0; i < BUTTON_COUNT; i++) {
            SDL_SetRenderDrawBlendMode(pRenderer, SDL_BLENDMODE_BLEND);
            if (i == selectedIndex)
                SDL_SetRenderDrawColor(pRenderer, 80, 80, 20, 180);
            else
                SDL_SetRenderDrawColor(pRenderer, 30, 30, 30, 160);
            SDL_RenderFillRect(pRenderer, &rects[i]);
            SDL_SetRenderDrawColor(pRenderer, 150, 150, 150, 255);
            SDL_RenderDrawRect(pRenderer, &rects[i]);

            SDL_Texture *tex = (i == selectedIndex) ? texSelected[i] : texNormal[i];
            int tw, th;
            SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
            SDL_Rect textRect = {
                rects[i].x + (rects[i].w - tw) / 2,
                rects[i].y + (rects[i].h - th) / 2,
                tw, th
            };
            SDL_RenderCopy(pRenderer, tex, NULL, &textRect);
        }

        SDL_RenderPresent(pRenderer);
    }

    for (int i = 0; i < BUTTON_COUNT; i++) {
        SDL_DestroyTexture(texNormal[i]);
        SDL_DestroyTexture(texSelected[i]);
    }
    if (bgTexture) SDL_DestroyTexture(bgTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);

    // NE PAS appeler SDL_Quit/TTF_Quit/IMG_Quit ici
    // Le main s'en charge après

    return result;
}