#pragma once
#include <SDL2/SDL.h>
#include "../SelectionManager/SelectionManager.hpp"
#include "../Renderer/Renderer.hpp"

class EventManager {
    SDL_Event         event    {};
    SelectionManager* selMgr   { nullptr };
    Renderer*         renderer { nullptr };
    bool              quit     { false   };
    bool              dragging { false   };
    int dragStartX{ 0 }, dragStartY{ 0 };
    int dragStartOffsetX{ 0 }, dragStartOffsetY{ 0 };
public:
    EventManager(SelectionManager& selMgr, Renderer& renderer);
    void pollEvents();
    bool isQuit() const;
private:
    void onMouseDown(int x, int y, int btn);
    void onMouseUp  (int x, int y, int btn);
    void onMouseMotion(int x, int y);
    void onMouseWheel(int x, int y, int direction);
    void onKeyDown  (SDL_Keycode key);
};