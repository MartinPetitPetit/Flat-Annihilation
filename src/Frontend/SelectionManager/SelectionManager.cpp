#include "SelectionManager.hpp"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr int LOCAL_PLAYER_TEAM = 0;

    bool isSelectableByLocalPlayer(const Unit* unit)
    {
        return unit != nullptr && unit->getTeam() == LOCAL_PLAYER_TEAM;
    }

    SDL_Point unitScreenPos(const Unit* unit, int offsetX, int offsetY, int scale)
    {
        return {
            offsetX + unit->getPos().getX() * scale + scale / 2,
            offsetY + unit->getPos().getY() * scale + scale / 2
        };
    }
}

void SelectionManager::startDrag(int x, int y)
{
    isDragging = true;
    dragStart  = { x, y };
    dragRect   = { x, y, 0, 0 };
}

void SelectionManager::updateDrag(int x, int y)
{
    if (!isDragging) {
        return;
    }

    dragRect.x = std::min(x, dragStart.x);
    dragRect.y = std::min(y, dragStart.y);
    dragRect.w = std::abs(x - dragStart.x);
    dragRect.h = std::abs(y - dragStart.y);
}

void SelectionManager::endDrag(
    int x,
    int y,
    std::vector<Unit*>& units,
    int offsetX,
    int offsetY,
    int scale
) {
    updateDrag(x, y);
    isDragging = false;

    /*
     * Si le rectangle est trop petit, on considère que c'est un clic simple.
     */
    if (dragRect.w < 4 && dragRect.h < 4) {
        tryClickSelect(x, y, units, offsetX, offsetY, scale);
        return;
    }

    for (Unit* unit : selected) {
        if (unit != nullptr) {
            unit->setSelected(false);
        }
    }

    selected.clear();

    /*
     * Le joueur humain ne peut sélectionner que ses propres unités.
     * Les collecteurs et soldats de l'IA restent visibles, mais non contrôlables.
     */
    for (Unit* unit : units) {
        if (!isSelectableByLocalPlayer(unit)) {
            continue;
        }

        SDL_Point p = unitScreenPos(unit, offsetX, offsetY, scale);

        if (p.x >= dragRect.x && p.x <= dragRect.x + dragRect.w &&
            p.y >= dragRect.y && p.y <= dragRect.y + dragRect.h) {
            unit->setSelected(true);
            selected.push_back(unit);
        }
    }
}

void SelectionManager::tryClickSelect(
    int x,
    int y,
    std::vector<Unit*>& units,
    int offsetX,
    int offsetY,
    int scale
) {
    for (Unit* unit : selected) {
        if (unit != nullptr) {
            unit->setSelected(false);
        }
    }

    selected.clear();

    int threshold = std::max(8, scale / 2 + 2);

    /*
     * Le clic simple ignore aussi les unités ennemies.
     */
    for (Unit* unit : units) {
        if (!isSelectableByLocalPlayer(unit)) {
            continue;
        }

        SDL_Point p = unitScreenPos(unit, offsetX, offsetY, scale);
        int dx = p.x - x;
        int dy = p.y - y;

        if (dx * dx + dy * dy <= threshold * threshold) {
            unit->setSelected(true);
            selected.push_back(unit);
            break;
        }
    }
}

void SelectionManager::deleteSelected(std::vector<std::unique_ptr<Unit>>& units)
{
    for (Unit* unit : selected) {
        if (unit != nullptr) {
            unit->setSelected(false);
        }

        units.erase(
            std::remove_if(
                units.begin(),
                units.end(),
                [unit](const std::unique_ptr<Unit>& ptr) {
                    return ptr.get() == unit;
                }
            ),
            units.end()
        );
    }

    selected.clear();
}

std::vector<Unit*>& SelectionManager::getSelected()
{
    return selected;
}

void SelectionManager::clearSelection()
{
    for (Unit* unit : selected) {
        if (unit != nullptr) {
            unit->setSelected(false);
        }
    }

    selected.clear();
}

SDL_Rect SelectionManager::getDragRect() const
{
    return dragRect;
}

bool SelectionManager::getIsDragging() const
{
    return isDragging;
}
