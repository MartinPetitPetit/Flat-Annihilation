/*-SDL_Event event

-SelectionManager* selMgr

-bool quit

+EventManager(selMgr SelectionManager&)

+pollEvents() : void

+isQuit() : bool

-onMouseDown(x,y,btn) : void

-onMouseUp(x,y,btn) : void

-onKeyDown(key) : void*/

#include "EventManager.hpp"

EventManager::EventManager(SelectionManager& s, Renderer& r, UIManager& u)
    : selMgr(&s), renderer(&r), uiManager(&u) {}

void EventManager::pollEvents()
{
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            quit = true;
            break;
        case SDL_MOUSEBUTTONDOWN:
            onMouseDown(event.button.x, event.button.y, event.button.button);
            break;
        case SDL_MOUSEBUTTONUP:
            onMouseUp(event.button.x, event.button.y, event.button.button);
            break;
        case SDL_MOUSEMOTION:
            onMouseMotion(event.motion.x, event.motion.y);
            break;
        case SDL_MOUSEWHEEL: {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            onMouseWheel(mx, my, event.wheel.y);
            break;
        }
        case SDL_KEYDOWN:
            onKeyDown(event.key.keysym.sym);
            break;
        }
    }
}

bool EventManager::isQuit() const { return quit; }

void EventManager::onMouseDown(int x, int y, int btn)
{
    if (btn == SDL_BUTTON_LEFT) {
        if (uiManager->handleHUDClick(x, y)) return;

        // Mode placement bâtiment
        if (uiManager->isInBuildingMode()) {
            // Le placement réel est délégué à Game via un flag
            // On notifie juste via un membre dédié
            pendingBuildX = x;
            pendingBuildY = y;
            pendingBuild  = true;
            return;
        }

        selMgr->startDrag(x, y);
    }

    if (btn == SDL_BUTTON_RIGHT) {
        uiManager->cancelBuildingMode();
        dragging         = true;
        dragStartX       = x;
        dragStartY       = y;
        dragStartOffsetX = renderer->getOffsetX();
        dragStartOffsetY = renderer->getOffsetY();
    }
}
void EventManager::onMouseUp(int x, int y, int btn)
{
    if (btn == SDL_BUTTON_LEFT) {
        std::vector<Unit*> allUnits;
        selMgr->endDrag(x, y, allUnits);
    }
    if (btn == SDL_BUTTON_RIGHT)
        dragging = false;
}

void EventManager::onMouseMotion(int x, int y)
{
    if (selMgr->getIsDragging())
        selMgr->updateDrag(x, y);

    if (dragging)
        renderer->setOffset(
            dragStartOffsetX + (x - dragStartX),
            dragStartOffsetY + (y - dragStartY)
        );
}

void EventManager::onMouseWheel(int x, int y, int direction)
{
    renderer->applyZoom(x, y, direction);
}

void EventManager::onKeyDown(SDL_Keycode key)
{
    if (key == SDLK_ESCAPE) quit = true;
}