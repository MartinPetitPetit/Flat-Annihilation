/*-Renderer* renderer

-ResourceManager* resMgr

+renderHUD(player) : void

+renderMinimap(map,players) : void

+renderHealthBar(entity) : void

+renderSelectionPanel(selected) : void

+renderProductionQueue(b) : void

+renderConstructionBar(b) : void*/

#include "UIManager.hpp"
#include <SDL2/SDL_image.h>
#include <cstring>

UIManager::UIManager(Renderer& r) : renderer(&r) {}

void UIManager::drawButton(const char* label, SDL_Rect rect, bool selected)
{
    SDL_Color bg  = selected ? SDL_Color{80,80,20,180} : SDL_Color{30,30,30,160};
    SDL_Color brd = { 150, 150, 150, 255 };
    renderer->drawRect(rect, bg,  true);
    renderer->drawRect(rect, brd, false);

    // Texte centré
    SDL_Color col = selected ? SDL_Color{255,220,0,255} : SDL_Color{200,200,200,255};
    SDL_Surface* surf = TTF_RenderText_Blended(renderer->getFont(), label, col);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer->getSDLRenderer(), surf);
    SDL_Rect dst = {
        rect.x + (rect.w - surf->w) / 2,
        rect.y + (rect.h - surf->h) / 2,
        surf->w, surf->h
    };
    renderer->drawTexture(tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

int UIManager::showMainMenu(DISPLAY_OPTIONS& options)
{
    const int   N      = 4;
    const char* labels[N] = { "Generate Map", "Solo vs IA", "Multiplayer", "Options" };
    int btnW = 320, btnH = 50;
    int startY = options.height / 2 - (N * (btnH + 15)) / 2;
    SDL_Rect rects[N];
    for (int i = 0; i < N; i++)
        rects[i] = { options.width/2 - btnW/2, startY + i*(btnH+15), btnW, btnH };

    // Fond
    SDL_Texture* bg = nullptr;
    SDL_Surface* s  = IMG_Load("background.png");
    if (s) { bg = SDL_CreateTextureFromSurface(renderer->getSDLRenderer(), s); SDL_FreeSurface(s); }

    SDL_Event ev;
    bool isOpen = true;
    int  sel    = 0;
    int  result = -1;

    while (isOpen) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT: isOpen = false; break;
            case SDL_KEYDOWN:
                if (ev.key.keysym.sym == SDLK_UP)    { sel--; if (sel < 0) sel = N-1; }
                if (ev.key.keysym.sym == SDLK_DOWN)  { sel++; if (sel >= N) sel = 0; }
                if (ev.key.keysym.sym == SDLK_RETURN) {
                    if (sel == 3) { showOptionsMenu(options); }
                    else { result = sel; isOpen = false; }
                }
                if (ev.key.keysym.sym == SDLK_ESCAPE) isOpen = false;
                break;
            case SDL_MOUSEMOTION:
                for (int i = 0; i < N; i++)
                    if (ev.motion.x >= rects[i].x && ev.motion.x <= rects[i].x+rects[i].w &&
                        ev.motion.y >= rects[i].y && ev.motion.y <= rects[i].y+rects[i].h)
                        sel = i;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT)
                    for (int i = 0; i < N; i++)
                        if (ev.button.x >= rects[i].x && ev.button.x <= rects[i].x+rects[i].w &&
                            ev.button.y >= rects[i].y && ev.button.y <= rects[i].y+rects[i].h) {
                            if (i == 3) showOptionsMenu(options);
                            else { result = i; isOpen = false; }
                        }
                break;
            }
        }

        renderer->clear();
        if (bg) renderer->drawTexture(bg, NULL, NULL);
        for (int i = 0; i < N; i++) drawButton(labels[i], rects[i], i == sel);
        renderer->present();
    }

    if (bg) SDL_DestroyTexture(bg);
    return result;
}

void UIManager::showOptionsMenu(DISPLAY_OPTIONS& options)
{
    const int RES_COUNT = 4;
    const char* res_labels[RES_COUNT] = { "800x600","1280x720","1920x1080","2560x1440" };
    const int   res_w[RES_COUNT]      = { 800, 1280, 1920, 2560 };
    const int   res_h[RES_COUNT]      = { 600,  720, 1080, 1440 };

    int currentRes = 0;
    for (int i = 0; i < RES_COUNT; i++)
        if (res_w[i] == options.width && res_h[i] == options.height) currentRes = i;
    bool fullscreen = options.fullscreen;

    const int   M      = 3;
    const char* labels[M] = { "Resolution", "Fullscreen", "Back" };
    int btnW = 460, btnH = 50;
    int startY = options.height/2 - (M*(btnH+15))/2;
    SDL_Rect rects[M];
    for (int i = 0; i < M; i++)
        rects[i] = { options.width/2 - btnW/2, startY + i*(btnH+15), btnW, btnH };

    SDL_Event ev;
    bool isOpen = true;
    int  sel    = 0;

    while (isOpen) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT: isOpen = false; break;
            case SDL_KEYDOWN:
                if (ev.key.keysym.sym == SDLK_UP)    { sel--; if (sel<0) sel=M-1; }
                if (ev.key.keysym.sym == SDLK_DOWN)  { sel++; if (sel>=M) sel=0; }
                if (ev.key.keysym.sym == SDLK_LEFT)  {
                    if (sel==0) { currentRes--; if (currentRes<0) currentRes=RES_COUNT-1; }
                    if (sel==1) fullscreen = !fullscreen;
                }
                if (ev.key.keysym.sym == SDLK_RIGHT) {
                    if (sel==0) { currentRes++; if (currentRes>=RES_COUNT) currentRes=0; }
                    if (sel==1) fullscreen = !fullscreen;
                }
                if (ev.key.keysym.sym == SDLK_RETURN && sel==2) isOpen = false;
                if (ev.key.keysym.sym == SDLK_ESCAPE) isOpen = false;
                break;
            case SDL_MOUSEMOTION:
                for (int i = 0; i < M; i++)
                    if (ev.motion.x >= rects[i].x && ev.motion.x <= rects[i].x+rects[i].w &&
                        ev.motion.y >= rects[i].y && ev.motion.y <= rects[i].y+rects[i].h)
                        sel = i;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT)
                    for (int i = 0; i < M; i++)
                        if (ev.button.x >= rects[i].x && ev.button.x <= rects[i].x+rects[i].w &&
                            ev.button.y >= rects[i].y && ev.button.y <= rects[i].y+rects[i].h) {
                            if (i==0) currentRes = (currentRes+1) % RES_COUNT;
                            if (i==1) fullscreen = !fullscreen;
                            if (i==2) isOpen = false;
                        }
                break;
            }
        }

        renderer->clear();
        for (int i = 0; i < M; i++) {
            drawButton(labels[i], rects[i], i == sel);

            // Valeur à droite
            char val[64] = "";
            if (i==0) snprintf(val, sizeof(val), "< %s >", res_labels[currentRes]);
            if (i==1) snprintf(val, sizeof(val), "< %s >", fullscreen ? "ON" : "OFF");
            if (val[0]) {
                SDL_Color cyan = { 0, 220, 255, 255 };
                SDL_Surface* vs = TTF_RenderText_Blended(renderer->getFont(), val, cyan);
                SDL_Texture* vt = SDL_CreateTextureFromSurface(renderer->getSDLRenderer(), vs);
                SDL_Rect vr = { rects[i].x+rects[i].w-vs->w-15, rects[i].y+(rects[i].h-vs->h)/2, vs->w, vs->h };
                renderer->drawTexture(vt, NULL, &vr);
                SDL_FreeSurface(vs);
                SDL_DestroyTexture(vt);
            }
        }
        renderer->present();
    }

    options.width      = res_w[currentRes];
    options.height     = res_h[currentRes];
    options.fullscreen = fullscreen;
}

void UIManager::renderHUD()
{
    // TODO
}

void UIManager::renderMinimap(MAP& map, int MAP_W, int MAP_H)
{
    // TODO
}

void UIManager::renderSelectionPanel()
{
    // TODO
}

void UIManager::renderDragRect(SelectionManager& sel)
{
    if (!sel.getIsDragging()) return;
    renderer->drawRect(sel.getDragRect(), { 255, 255, 255, 120 }, false);
}