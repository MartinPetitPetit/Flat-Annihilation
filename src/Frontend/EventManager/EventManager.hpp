#pragma once
#include <SDL2/SDL.h>
#include "../SelectionManager/SelectionManager.hpp"
#include "../Renderer/Renderer.hpp"
#include "../UIManager/UIManager.hpp"
#include "../../Backend/Unit/Unit.hpp"
#include <vector>
#include <memory>

class EventManager {
    SDL_Event         event    {};
    SelectionManager* selMgr     { nullptr };
    Renderer*         renderer   { nullptr };
    UIManager*        uiManager  { nullptr };
    bool              quit     { false   };
    bool              dragging { false   };
    int dragStartX{ 0 }, dragStartY{ 0 };
    int dragStartLeftX{ 0 }, dragStartLeftY{ 0 };
    int dragStartOffsetX{ 0 }, dragStartOffsetY{ 0 };

    // Référence vers les unités de la scène
    std::vector<std::unique_ptr<Unit>>* units { nullptr };

public:
    EventManager(SelectionManager& selMgr, Renderer& renderer,
                 UIManager& uiManager,
                 std::vector<std::unique_ptr<Unit>>& units);
    void pollEvents();
    bool isQuit() const;

    bool pendingBuild  { false };
    int  pendingBuildX { 0     };
    int  pendingBuildY { 0     };
    void consumeBuild() { pendingBuild = false; }
    bool pendingMove  { false };
    int  pendingMoveX { 0     };
    int  pendingMoveY { 0     };
    void consumeMove() { pendingMove = false; }
    bool pendingBuildingSelect  { false };
    int  pendingBuildingSelectX { 0 };
    int  pendingBuildingSelectY { 0 };
    void consumeBuildingSelect() { pendingBuildingSelect = false; }

private:
    void onMouseDown(int x, int y, int btn);
    void onMouseUp  (int x, int y, int btn);
    void onMouseMotion(int x, int y);
    void onMouseWheel(int x, int y, int direction);
    void onKeyDown  (SDL_Keycode key);
};