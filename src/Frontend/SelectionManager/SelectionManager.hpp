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

    // Sélection par drag sur la liste d'unités de la scène
    void endDrag(int x, int y, std::vector<Unit*>& units,
                 int offsetX, int offsetY, int scale);

    // Clic simple
    void tryClickSelect(int x, int y, std::vector<Unit*>& units,
                        int offsetX, int offsetY, int scale);

    // Supprime les unités sélectionnées de la scène
    void deleteSelected(std::vector<std::unique_ptr<Unit>>& units);

    std::vector<Unit*>& getSelected();
    void clearSelection();
    SDL_Rect getDragRect()   const;
    bool     getIsDragging() const;
};