#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "../Window/Window.hpp"
#include "../../Backend/Unit/Unit.hpp"

class SelectionManager {
    std::vector<Unit*> selected;
    SDL_Rect           dragRect  {};
    bool               isDragging{ false };
    Vector2i           dragStart {};
public:
    void startDrag(int x, int y);
    void updateDrag(int x, int y);
    void endDrag(int x, int y, std::vector<Unit*>& units);
    std::vector<Unit*>& getSelected();
    void clearSelection();
    SDL_Rect getDragRect()  const;
    bool     getIsDragging() const;
};