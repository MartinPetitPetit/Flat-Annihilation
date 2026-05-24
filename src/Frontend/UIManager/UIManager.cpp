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

UIManager::UIManager(Renderer& r, Window& w) : renderer(&r), window(&w) {}

void UIManager::setHUDStats(int fps, int tps, Uint64 tick, int tickRateVal)
{
    currentFPS = fps;
    currentTPS = tps;
    gameTick   = tick;
    tickRate   = tickRateVal;
}

bool UIManager::isGamePaused() const { return gamePaused; }

// ---------- Helpers HUD ----------

void UIManager::drawHUDRect(SDL_Rect r, SDL_Color fill, SDL_Color border)
{
    renderer->drawRect(r, fill,   true);
    renderer->drawRect(r, border, false);
}

void UIManager::drawHUDText(const char* text, int x, int y, SDL_Color color)
{
    if (!renderer->getFont()) return;
    SDL_Surface* surf = TTF_RenderText_Blended(renderer->getFont(), text, color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer->getSDLRenderer(), surf);
    SDL_Rect dst = { x, y, surf->w, surf->h };
    renderer->drawTexture(tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

// ---------- Rects HUD (positions relatives à la fenêtre) ----------

SDL_Rect UIManager::getHUDPauseRect() const
{
    int w = window->getOptions().width;
    return { w - 160, 60, 70, 30 };
}

SDL_Rect UIManager::getHUDStopRect() const
{
    int w = window->getOptions().width;
    return { w - 85, 60, 70, 30 };
}

SDL_Rect UIManager::getHUDSelectionRect() const
{
    int h = window->getOptions().height;
    return { 0, h - 160, 280, 160 };
}

SDL_Rect UIManager::getHUDBuildingRect() const
{
    int w = window->getOptions().width;
    int h = window->getOptions().height;
    return { 282, h - 70, w - 284, 70 };
}
void UIManager::drawButton(const char* label, SDL_Rect rect, bool selected, bool disabled)
{
    SDL_Color bg  = disabled  ? SDL_Color{20,20,20,120}
                  : selected  ? SDL_Color{80,80,20,180}
                              : SDL_Color{30,30,30,160};
    SDL_Color brd = disabled  ? SDL_Color{70,70,70,180}
                              : SDL_Color{150,150,150,255};
    renderer->drawRect(rect, bg,  true);
    renderer->drawRect(rect, brd, false);

    SDL_Color col = disabled  ? SDL_Color{80,80,80,180}
                  : selected  ? SDL_Color{255,220,0,255}
                              : SDL_Color{200,200,200,255};
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

int UIManager::showMainMenu(DISPLAY_OPTIONS& options, Sound& sound)
{
    const int   N      = 4;
    const char* labels[N] = { "generate map", "solo vs ia", "multiplayer", "options" };
    int btnW = 320, btnH = 50;
	SDL_Rect rects[N]; // sera recalculé à chaque frame

    // Fond
    SDL_Texture* bg = nullptr;
    SDL_Surface* s  = IMG_Load("background.png");
    if (s) { bg = SDL_CreateTextureFromSurface(renderer->getSDLRenderer(), s); SDL_FreeSurface(s); }

    SDL_Event ev;
    bool isOpen = true;
    int  sel    = 0;
    int  result = -1;

    while (isOpen) {
		int startY = options.height / 2 - (N * (btnH + 15)) / 2;
		for (int i = 0; i < N; i++)
			rects[i] = { options.width/2 - btnW/2, startY + i*(btnH+15), btnW, btnH };
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT: isOpen = false; break;
            case SDL_KEYDOWN:
                if (ev.key.keysym.sym == SDLK_UP)    { sel--; if (sel < 0) sel = N-1; }
                if (ev.key.keysym.sym == SDLK_DOWN)  { sel++; if (sel >= N) sel = 0; }
                if (ev.key.keysym.sym == SDLK_RETURN) {
                    if (sel == 3) { showOptionsMenu(options, sound); }
                    else { result = sel; isOpen = false; }
                }
                if (ev.key.keysym.sym == SDLK_ESCAPE) isOpen = false;
                break;
			case SDL_MOUSEMOTION:
				for (int i = 0; i < N; i++) {
					int mx = ev.motion.x, my = ev.motion.y;
					if (mx >= rects[i].x && mx <= rects[i].x + rects[i].w &&
						my >= rects[i].y && my <= rects[i].y + rects[i].h && sel != i) {
						sound.play("hover");
						sel = i;
					}
				}
				break;
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT) {
                    int mx = ev.button.x, my = ev.button.y;
                    for (int i = 0; i < N; i++) {
                        if (mx >= rects[i].x && mx <= rects[i].x + rects[i].w &&
                            my >= rects[i].y && my <= rects[i].y + rects[i].h) {
                            sound.play("click");
                            if (i == 3) showOptionsMenu(options, sound);
                            else { result = i; isOpen = false; }
                        }
                    }
                }
                break;
			}
		}
        renderer->clear();
        if (bg) renderer->drawTexture(bg, NULL, NULL);
        for (int i = 0; i < N; i++) drawButton(labels[i], rects[i], i == sel,false);
        renderer->present();
    }

    if (bg) SDL_DestroyTexture(bg);
    return result;
	
}

void UIManager::showOptionsMenu(DISPLAY_OPTIONS& options,Sound& sound)
{
    const int RES_COUNT = 5;
    const char* res_labels[RES_COUNT] = { "200x200","800x600","1280x720","1920x1080","2560x1440" };
    const int   res_w[RES_COUNT]      = { 600, 800, 1280, 1920, 2560 };
    const int   res_h[RES_COUNT]      = { 600, 600,  720, 1080, 1440 };

    int currentRes = 0;
    for (int i = 0; i < RES_COUNT; i++)
        if (res_w[i] == options.width && res_h[i] == options.height) currentRes = i;
    bool fullscreen = options.fullscreen;

    const int   M      = 3;
    const char* labels[M] = { "resolution", "fullscreen", "back" };
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
				if (ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_RIGHT) {
					if (sel==0 && !fullscreen) {  // ← bloqué si fullscreen
						if (ev.key.keysym.sym == SDLK_LEFT) { currentRes--; if (currentRes<0) currentRes=RES_COUNT-1; }
						else                                 { currentRes++; if (currentRes>=RES_COUNT) currentRes=0; }
						applyResolution(options, res_w[currentRes], res_h[currentRes], fullscreen);
						startY = options.height/2 - (M*(btnH+15))/2;
						for (int j = 0; j < M; j++)
							rects[j] = { options.width/2 - btnW/2, startY + j*(btnH+15), btnW, btnH };
					}
					if (sel==1) {
						fullscreen = !fullscreen;
						applyResolution(options, res_w[currentRes], res_h[currentRes], fullscreen);
					}
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
                            if (i==0 && !fullscreen) {  // ← bloqué si fullscreen
								currentRes = (currentRes+1) % RES_COUNT;
								applyResolution(options, res_w[currentRes], res_h[currentRes], fullscreen);
								startY = options.height/2 - (M*(btnH+15))/2;
								for (int j = 0; j < M; j++)
									rects[j] = { options.width/2 - btnW/2, startY + j*(btnH+15), btnW, btnH };
							}
							if (i==1) {
								fullscreen = !fullscreen;
								applyResolution(options, res_w[currentRes], res_h[currentRes], fullscreen);
							}
							if (i==2) isOpen = false;
                        }
                break;
            }
        }

        renderer->clear();
		for (int i = 0; i < M; i++) {
			bool disabled = (i == 0 && fullscreen); // résolution grisée si fullscreen
			drawButton(labels[i], rects[i], i == sel && !disabled, disabled);

            // Valeur à droite
            char val[64] = "";
            if (i==0) snprintf(val, sizeof(val), " %s ", res_labels[currentRes]);
            if (i==1) snprintf(val, sizeof(val), " %s ", fullscreen ? "on" : "off");
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
}


void UIManager::renderHUD(const Player* localPlayer)
{
    int w = window->getOptions().width;

    SDL_Color transparent  = {   0,   0,   0, 140 };
    SDL_Color borderCyan   = {   0, 220, 255, 255 };
    SDL_Color borderYellow = { 255, 220,   0, 255 };
    SDL_Color borderRed    = { 255,  60,  60, 255 };
    SDL_Color borderGreen  = {  60, 200,  60, 255 };
    SDL_Color borderWhite  = { 255, 255, 255, 220 };

    // --- FPS ---
    SDL_Rect fpsBox = { 4, 4, 130, 50 };
    drawHUDRect(fpsBox, transparent, borderCyan);
    char fps_str[32]; snprintf(fps_str, sizeof(fps_str), "fps: %d", currentFPS);
    char tps_str[32]; snprintf(tps_str, sizeof(tps_str), "tps: %d", currentTPS);
    drawHUDText(fps_str, 10, 10, { 0, 220, 255, 255 });
    drawHUDText(tps_str, 10, 30, { 0, 220, 255, 255 });

    // --- Ressources joueur ---
    if (localPlayer) {
        SDL_Rect resBox = { 140, 4, 160, 25 };
        drawHUDRect(resBox, transparent, { 160, 120, 60, 255 });
        char wood_str[32];
        snprintf(wood_str, sizeof(wood_str), "wood: %d", localPlayer->getWood());
        drawHUDText(wood_str, 148, 8, { 200, 180, 100, 255 });
    }

    // --- Game Time ---
    float gameTimeSec = static_cast<float>(gameTick) / (tickRate > 0 ? tickRate : 1);
    int minutes = static_cast<int>(gameTimeSec) / 60;
    int seconds = static_cast<int>(gameTimeSec) % 60;
    char time_str[32]; snprintf(time_str, sizeof(time_str), "%02d:%02d", minutes, seconds);
    SDL_Rect timeBox = { w - 165, 4, 161, 50 };
    drawHUDRect(timeBox, transparent, borderYellow);
    drawHUDText("game time", w - 155, 8,  { 255, 220, 0, 255 });
    drawHUDText(time_str,    w - 135, 28, { 255, 255, 255, 255 });

    // --- Bouton PAUSE ---
    SDL_Rect pauseRect = getHUDPauseRect();
    SDL_Color pauseColor = gamePaused ? SDL_Color{255,180,0,200} : SDL_Color{40,40,40,180};
    drawHUDRect(pauseRect, pauseColor, borderYellow);
    drawHUDText(gamePaused ? "resume" : "pause", pauseRect.x + 6, pauseRect.y + 7, { 255, 220, 0, 255 });

    // --- Bouton STOP ---
    SDL_Rect stopRect = getHUDStopRect();
    drawHUDRect(stopRect, { 120, 0, 0, 180 }, borderRed);
    drawHUDText("stop", stopRect.x + 18, stopRect.y + 7, { 255, 60, 60, 255 });

    // --- Panneau sélection ---
    SDL_Rect selRect = getHUDSelectionRect();
    drawHUDRect(selRect, { 10, 10, 30, 180 }, borderWhite);
    drawHUDText("selected units", selRect.x + 8, selRect.y + 8,  { 200, 200, 200, 255 });
    drawHUDText("summary",        selRect.x + 8, selRect.y + 30, { 150, 150, 150, 255 });
    SDL_Rect iconPlaceholder = { selRect.x + 8, selRect.y + 55, selRect.w - 16, selRect.h - 65 };
    drawHUDRect(iconPlaceholder, { 30, 30, 60, 120 }, { 80, 80, 120, 255 });
    drawHUDText("(empty)", selRect.x + 90, selRect.y + 90, { 80, 80, 100, 255 });

    // --- Building bar ---
    SDL_Rect buildRect = getHUDBuildingRect();
    drawHUDRect(buildRect, { 10, 30, 10, 180 }, borderGreen);

    // Bouton Town Center
    SDL_Rect tcBtn = { buildRect.x + 8, buildRect.y + 10, 90, 50 };
    bool tcSelected = inBuildingMode && selectedBuildingType == BuildingType::TownCenter;
    SDL_Color tcFill = tcSelected ? SDL_Color{80,80,20,220} : SDL_Color{20,60,20,180};
    drawHUDRect(tcBtn, tcFill, tcSelected ? SDL_Color{255,220,0,255} : borderGreen);
    drawHUDText("TC",         tcBtn.x + 32, tcBtn.y + 8,  { 100, 255, 100, 255 });
    drawHUDText("w:100",      tcBtn.x + 14, tcBtn.y + 28, { 180, 140, 80,  255 });
}
bool UIManager::isInBuildingMode() const { return inBuildingMode; }
BuildingType UIManager::getSelectedBuildingType() const { return selectedBuildingType; }
void UIManager::cancelBuildingMode() { inBuildingMode = false; }

void UIManager::renderBuildingGhost(int mouseX, int mouseY, BuildingType type,
                                    int scale, int offsetX, int offsetY)
{
    const BuildingDef& def = getBuildingDef(type);

    // Convertir position souris -> cellule
    int cellX = (mouseX - offsetX) / scale;
    int cellY = (mouseY - offsetY) / scale;

    SDL_Rect ghost;
    ghost.x = offsetX + cellX * scale;
    ghost.y = offsetY + cellY * scale;
    ghost.w = def.sizeX * scale;
    ghost.h = def.sizeY * scale;

    renderer->drawRect(ghost, { 255, 220, 0, 80  }, true);
    renderer->drawRect(ghost, { 255, 220, 0, 200 }, false);
}

void UIManager::renderBuildings(const MAP& map, const std::vector<Player*>& players,
                                int scale, int offsetX, int offsetY)
{
    for (const Player* p : players) {
        if (!p) continue;
        for (const auto& b : p->getBuildings()) {
            if (!b->isAlive()) continue;
            const BuildingDef& def = getBuildingDef(b->getType());

            SDL_Rect r;
            r.x = offsetX + b->getMapX() * scale;
            r.y = offsetY + b->getMapY() * scale;
            r.w = def.sizeX * scale;
            r.h = def.sizeY * scale;

            // Couleur selon propriétaire (bleu joueur 0, rouge IA)
            SDL_Color fill   = (b->getTeam() == 0)
                               ? SDL_Color{ 30, 60, 180, 200 }
                               : SDL_Color{ 180, 30, 30, 200 };
            SDL_Color border = (b->getTeam() == 0)
                               ? SDL_Color{ 100, 160, 255, 255 }
                               : SDL_Color{ 255, 100, 100, 255 };

            renderer->drawRect(r, fill,   true);
            renderer->drawRect(r, border, false);

            if (scale >= 8) {
                drawHUDText(def.name.c_str(), r.x + 2, r.y + 2, { 220, 220, 220, 255 });
            }
        }
    }
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
bool UIManager::handleHUDClick(int mx, int my)
{
    SDL_Rect pauseRect = getHUDPauseRect();
    SDL_Rect stopRect  = getHUDStopRect();

    auto inRect = [](int x, int y, SDL_Rect r) {
        return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
    };

    if (inRect(mx, my, pauseRect)) { gamePaused = !gamePaused; return true; }
    if (inRect(mx, my, stopRect))  { return true; }

    // Bouton Town Center dans la building bar
    SDL_Rect buildRect = getHUDBuildingRect();
    SDL_Rect tcBtn = { buildRect.x + 8, buildRect.y + 10, 90, 50 };
    if (inRect(mx, my, tcBtn)) {
        if (inBuildingMode && selectedBuildingType == BuildingType::TownCenter)
            inBuildingMode = false;  // désélectionne si déjà actif
        else {
            inBuildingMode       = true;
            selectedBuildingType = BuildingType::TownCenter;
        }
        return true;
    }

    return false;
}

void UIManager::applyResolution(DISPLAY_OPTIONS& options, int w, int h, bool fullscreen)
{
    printf("applyResolution appelé : %dx%d fs=%d\n", w, h, fullscreen); // ← debug
    bool fsChanged     = (fullscreen != options.fullscreen);
    options.width      = w;
    options.height     = h;
    options.fullscreen = fullscreen;
    window->resize(w, h);
    if (fsChanged)
        window->setFullscreen(fullscreen);
    renderer->updateViewport(w, h);
    printf("après resize, window dit : %dx%d\n", window->getOptions().width, window->getOptions().height); // ← debug
}