/*
 * Frontend/EventManager/EventManager.cpp
 *
 * Rôle du fichier :
 * Reads SDL events and converts mouse, keyboard, camera, build, movement, and attack-move inputs into game commands.
 *
 * Notes de lecture :
 * Ce module transforme les entrées SDL en intentions de jeu : sélection, déplacement, construction, attaque et caméra.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#include "EventManager.hpp"

/*
 * Constructeur : garde des pointeurs vers les gestionnaires nécessaires.
 * L'EventManager ne possède pas ces objets, il les utilise seulement.
 */
EventManager::EventManager(
    SelectionManager& s,
    Renderer& r,
    UIManager& u,
    std::vector<std::unique_ptr<Unit>>& unitList
)
: selMgr(&s),
renderer(&r),
uiManager(&u),
units(&unitList)
{
}

/*
 * Lit tous les événements SDL disponibles et les redirige vers
 * les méthodes spécialisées selon leur type.
 */
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

            default:
                break;
        }
    }
}

/*
 * Indique au game loop si une demande de fermeture a été reçue.
 */
bool EventManager::isQuit() const
{
    return quit;
}

/*
 * Vérifie si la touche utilisée pour l'ordre offensif est maintenue.
 * Le test accepte A et Q pour mieux fonctionner avec QWERTY et AZERTY.
 */
bool EventManager::isAttackMoveModifierPressed() const
{
    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);

    /*
     * SDL_SCANCODE_A : touche A sur clavier QWERTY.
     * SDL_SCANCODE_Q : touche physique équivalente sur certains claviers AZERTY.
     */
    return keyboardState[SDL_SCANCODE_A] != 0 ||
    keyboardState[SDL_SCANCODE_Q] != 0;
}

/*
 * Gère le début d'une action souris : clic HUD, placement,
 * déplacement offensif, sélection ou déplacement de caméra.
 */
void EventManager::onMouseDown(int x, int y, int btn)
{
    if (btn == SDL_BUTTON_LEFT) {
        if (uiManager->handleHUDClick(x, y)) {
            return;
        }

        if (uiManager->isInBuildingMode()) {
            pendingBuildX = x;
            pendingBuildY = y;
            pendingBuild  = true;
            return;
        }

        /*
         * A + clic gauche : mode offensif.
         * On convertit la position écran en cellule de carte.
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

        dragStartLeftX = x;
        dragStartLeftY = y;
        selMgr->startDrag(x, y);
    }

    if (btn == SDL_BUTTON_RIGHT) {
        /*
         * Clic droit avec unités sélectionnées : déplacement normal.
         */
        if (!selMgr->getSelected().empty()) {
            int cellX = (x - renderer->getOffsetX()) / renderer->getScale();
            int cellY = (y - renderer->getOffsetY()) / renderer->getScale();

            pendingMoveX = cellX;
            pendingMoveY = cellY;
            pendingMove  = true;
            return;
        }

        /*
         * Sinon, clic droit déplace la caméra.
         */
        uiManager->cancelBuildingMode();

        dragging         = true;
        dragStartX       = x;
        dragStartY       = y;
        dragStartOffsetX = renderer->getOffsetX();
        dragStartOffsetY = renderer->getOffsetY();
    }
}

/*
 * Termine les actions souris : validation de sélection ou arrêt du drag caméra.
 */
void EventManager::onMouseUp(int x, int y, int btn)
{
    if (btn == SDL_BUTTON_LEFT) {
        /*
         * Si le clic gauche servait à une commande spéciale,
         * on ne lance pas de sélection au relâchement.
         */
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

        /*
         * Clic simple : possible sélection de bâtiment.
         */
        int dx = x - dragStartLeftX;
        int dy = y - dragStartLeftY;

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

/*
 * Met à jour le rectangle de sélection ou le déplacement de la caméra.
 */
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

/*
 * Transmet la molette au renderer pour zoomer autour de la souris.
 */
void EventManager::onMouseWheel(int x, int y, int direction)
{
    renderer->applyZoom(x, y, direction);
}

/*
 * Gère les raccourcis clavier globaux comme quitter ou supprimer.
 */
void EventManager::onKeyDown(SDL_Keycode key)
{
    if (key == SDLK_ESCAPE) {
        quit = true;
    }

    if (key == SDLK_DELETE || key == SDLK_KP_PERIOD) {
        selMgr->deleteSelected(*units);
    }
}
