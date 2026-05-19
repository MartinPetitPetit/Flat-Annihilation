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