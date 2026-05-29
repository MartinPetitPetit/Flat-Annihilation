#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <vector>

#include "../SelectionManager/SelectionManager.hpp"
#include "../Renderer/Renderer.hpp"
#include "../UIManager/UIManager.hpp"
#include "../../Backend/Unit/Unit.hpp"

class EventManager
{
private:
    SDL_Event event {};

    SelectionManager* selMgr    { nullptr };
    Renderer*         renderer  { nullptr };
    UIManager*        uiManager { nullptr };

    bool quit     { false };
    bool dragging { false };

    int dragStartX       { 0 };
    int dragStartY       { 0 };
    int dragStartLeftX   { 0 };
    int dragStartLeftY   { 0 };
    int dragStartOffsetX { 0 };
    int dragStartOffsetY { 0 };

    /*
     * Utilisé quand le clic gauche est consommé par une commande spéciale.
     * Exemple : A + clic gauche pour le mode offensif.
     */
    bool leftClickConsumedByCommand { false };

    // Référence vers les unités de la scène.
    std::vector<std::unique_ptr<Unit>>* units { nullptr };

public:
    EventManager(
        SelectionManager& selMgr,
        Renderer& renderer,
        UIManager& uiManager,
        std::vector<std::unique_ptr<Unit>>& units
    );

    void pollEvents();
    bool isQuit() const;

    bool pendingBuild  { false };
    int  pendingBuildX { 0 };
    int  pendingBuildY { 0 };
    void consumeBuild() { pendingBuild = false; }

    bool pendingMove  { false };
    int  pendingMoveX { 0 };
    int  pendingMoveY { 0 };
    void consumeMove() { pendingMove = false; }

    /*
     * A + clic gauche : ordre de déplacement offensif.
     */
    bool pendingOffensiveMove  { false };
    int  pendingOffensiveMoveX { 0 };
    int  pendingOffensiveMoveY { 0 };
    void consumeOffensiveMove() { pendingOffensiveMove = false; }

    bool pendingBuildingSelect  { false };
    int  pendingBuildingSelectX { 0 };
    int  pendingBuildingSelectY { 0 };
    void consumeBuildingSelect() { pendingBuildingSelect = false; }

private:
    void onMouseDown(int x, int y, int btn);
    void onMouseUp(int x, int y, int btn);
    void onMouseMotion(int x, int y);
    void onMouseWheel(int x, int y, int direction);
    void onKeyDown(SDL_Keycode key);

    bool isAttackMoveModifierPressed() const;
};
