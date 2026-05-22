/*-vector<Unit*> selected

-SDL_Rect dragRect

-bool isDragging

-Vector2i dragStart

+startDrag(x,y) : void

+updateDrag(x,y) : void

+endDrag(x,y,units) : void

+getSelected() : vector<Unit*>

+clearSelection() : void

+renderDragRect(r Renderer&) : void*/

#include "SelectionManager.hpp"
#include <algorithm>
#include <cmath>

void SelectionManager::startDrag(int x, int y)
{
    isDragging = true;
    dragStart  = { x, y };
    dragRect   = { x, y, 0, 0 };
}

void SelectionManager::updateDrag(int x, int y)
{
    if (!isDragging) return;
    dragRect.x = std::min(x, dragStart.x);
    dragRect.y = std::min(y, dragStart.y);
    dragRect.w = std::abs(x - dragStart.x);
    dragRect.h = std::abs(y - dragStart.y);
}

void SelectionManager::endDrag(int x, int y, std::vector<Unit*>& units)
{
    updateDrag(x, y);
    // TODO: sélectionner les unités dans dragRect
    isDragging = false;
}

std::vector<Unit*>& SelectionManager::getSelected()  { return selected;   }
void SelectionManager::clearSelection()               { selected.clear();  }
SDL_Rect SelectionManager::getDragRect()  const       { return dragRect;   }
bool     SelectionManager::getIsDragging() const      { return isDragging; }