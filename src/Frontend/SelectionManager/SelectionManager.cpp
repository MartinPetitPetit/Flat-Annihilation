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

// Coordonnées écran d'une unité
static SDL_Point unitScreenPos(const Unit* u, int offsetX, int offsetY, int scale)
{
    return {
        offsetX + u->getPos().getX() * scale + scale / 2,
        offsetY + u->getPos().getY() * scale + scale / 2
    };
}

void SelectionManager::endDrag(int x, int y, std::vector<Unit*>& units,
                                int offsetX, int offsetY, int scale)
{
    updateDrag(x, y);
    isDragging = false;

    // Si le rect est trop petit -> clic simple
    if (dragRect.w < 4 && dragRect.h < 4) {
        tryClickSelect(x, y, units, offsetX, offsetY, scale);
        return;
    }

    // Désélectionner tout
    for (Unit* u : selected) u->setSelected(false);
    selected.clear();

    // Sélectionner les unités dans le rect
    for (Unit* u : units) {
        SDL_Point p = unitScreenPos(u, offsetX, offsetY, scale);
        if (p.x >= dragRect.x && p.x <= dragRect.x + dragRect.w &&
            p.y >= dragRect.y && p.y <= dragRect.y + dragRect.h) {
            u->setSelected(true);
            selected.push_back(u);
        }
    }
}

void SelectionManager::tryClickSelect(int x, int y, std::vector<Unit*>& units,
                                       int offsetX, int offsetY, int scale)
{
    // Désélectionner tout
    for (Unit* u : selected) u->setSelected(false);
    selected.clear();

    int threshold = std::max(8, scale / 2 + 2);

    for (Unit* u : units) {
        SDL_Point p = unitScreenPos(u, offsetX, offsetY, scale);
        int dx = p.x - x;
        int dy = p.y - y;
        if (dx * dx + dy * dy <= threshold * threshold) {
            u->setSelected(true);
            selected.push_back(u);
            break; // un seul par clic simple
        }
    }
}

void SelectionManager::deleteSelected(std::vector<std::unique_ptr<Unit>>& units)
{
    for (Unit* u : selected) {
        u->setSelected(false);
        units.erase(
            std::remove_if(units.begin(), units.end(),
                [u](const std::unique_ptr<Unit>& ptr) { return ptr.get() == u; }),
            units.end()
        );
    }
    selected.clear();
}

std::vector<Unit*>& SelectionManager::getSelected()  { return selected;   }
void SelectionManager::clearSelection()
{
    for (Unit* u : selected) u->setSelected(false);
    selected.clear();
}
SDL_Rect SelectionManager::getDragRect()   const { return dragRect;   }
bool     SelectionManager::getIsDragging() const { return isDragging; }