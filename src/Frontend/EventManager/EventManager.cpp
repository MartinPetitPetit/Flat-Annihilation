#include "EventManager.hpp"

<<<<<<< HEAD
EventManager::EventManager(SelectionManager& s, Renderer& r,
                           UIManager& u,
                           std::vector<std::unique_ptr<Unit>>& unitList)
    : selMgr(&s), renderer(&r), uiManager(&u), units(&unitList) {}
=======
EventManager::EventManager(
    SelectionManager& s,
    Renderer& r,
    UIManager& u,
    std::vector<std::unique_ptr<Unit>>& unitList
)
    : selMgr(&s), renderer(&r), uiManager(&u), units(&unitList)
{
}
>>>>>>> origin/ptn-de-vibe-coding

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
            int mx = 0;
            int my = 0;
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

bool EventManager::isQuit() const
{
    return quit;
}

bool EventManager::isAttackMoveModifierPressed() const
{
    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
    return keyboardState[SDL_SCANCODE_A] != 0;
}

void EventManager::onMouseDown(int x, int y, int btn)
{
    if (btn == SDL_BUTTON_LEFT) {
<<<<<<< HEAD
        if (uiManager->handleHUDClick(x, y)) return;
=======
        if (uiManager->handleHUDClick(x, y)) {
            return;
        }
>>>>>>> origin/ptn-de-vibe-coding

        if (uiManager->isInBuildingMode()) {
            pendingBuildX = x;
            pendingBuildY = y;
            pendingBuild  = true;
            return;
        }
<<<<<<< HEAD
=======

        /*
         * A + clic gauche : ordre offensif.
         * On convertit directement écran -> cellule ici, comme pour le clic droit.
         */
        if (isAttackMoveModifierPressed() && !selMgr->getSelected().empty()) {
            int cellX = (x - renderer->getOffsetX()) / renderer->getScale();
            int cellY = (y - renderer->getOffsetY()) / renderer->getScale();

            pendingOffensiveMoveX = cellX;
            pendingOffensiveMoveY = cellY;
            pendingOffensiveMove  = true;

            leftClickConsumedByCommand = true;
            return;
        }

>>>>>>> origin/ptn-de-vibe-coding
        dragStartLeftX = x;
        dragStartLeftY = y;
        selMgr->startDrag(x, y);
    }

    if (btn == SDL_BUTTON_RIGHT) {
<<<<<<< HEAD
        // Si des unités sont sélectionnées → ordre de déplacement
        if (!selMgr->getSelected().empty()) {
            // Convertir position écran → cellule
=======
        // Si des unités sont sélectionnées → ordre de déplacement manuel.
        if (!selMgr->getSelected().empty()) {
>>>>>>> origin/ptn-de-vibe-coding
            int cellX = (x - renderer->getOffsetX()) / renderer->getScale();
            int cellY = (y - renderer->getOffsetY()) / renderer->getScale();

            pendingMoveX = cellX;
            pendingMoveY = cellY;
            pendingMove  = true;
            return;
        }

<<<<<<< HEAD
        // Sinon → déplacer la caméra
=======
        // Sinon → déplacer la caméra.
>>>>>>> origin/ptn-de-vibe-coding
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
<<<<<<< HEAD
        // Collecter les raw pointers
        std::vector<Unit*> rawUnits;
        for (auto& u : *units) rawUnits.push_back(u.get());

        selMgr->endDrag(x, y, rawUnits,
                        renderer->getOffsetX(),
                        renderer->getOffsetY(),
                        renderer->getScale());

        // Clic simple (pas un vrai drag)
        int dx = x - dragStartLeftX;
        int dy = y - dragStartLeftY;
=======
        if (leftClickConsumedByCommand) {
            leftClickConsumedByCommand = false;
            return;
        }

        std::vector<Unit*> rawUnits;

        for (auto& unit : *units) {
            rawUnits.push_back(unit.get());
        }

        selMgr->endDrag(
            x,
            y,
            rawUnits,
            renderer->getOffsetX(),
            renderer->getOffsetY(),
            renderer->getScale()
        );

        // Clic simple : potentielle sélection de bâtiment.
        int dx = x - dragStartLeftX;
        int dy = y - dragStartLeftY;

>>>>>>> origin/ptn-de-vibe-coding
        if (dx * dx + dy * dy < 16) {
            pendingBuildingSelectX = x;
            pendingBuildingSelectY = y;
            pendingBuildingSelect  = true;
        }
    }

    if (btn == SDL_BUTTON_RIGHT) {
        dragging = false;
    }
}

void EventManager::onMouseMotion(int x, int y)
{
    if (selMgr->getIsDragging()) {
        selMgr->updateDrag(x, y);
    }

    if (dragging) {
        renderer->setOffset(
            dragStartOffsetX + (x - dragStartX),
            dragStartOffsetY + (y - dragStartY)
        );
    }
}

void EventManager::onMouseWheel(int x, int y, int direction)
{
    renderer->applyZoom(x, y, direction);
}

void EventManager::onKeyDown(SDL_Keycode key)
{
<<<<<<< HEAD
    if (key == SDLK_ESCAPE) quit = true;
=======
    if (key == SDLK_ESCAPE) {
        quit = true;
    }
>>>>>>> origin/ptn-de-vibe-coding

    if (key == SDLK_DELETE || key == SDLK_KP_PERIOD) {
        selMgr->deleteSelected(*units);
    }
<<<<<<< HEAD
}
=======
}
>>>>>>> origin/ptn-de-vibe-coding
