#pragma once

#include "../Map/Map.hpp"
#include "../Coordinate/Coordinate.hpp"

class Unit;

namespace MovementRules
{
    bool isBlockedTerrain(TERRAIN terrain);

    bool isFreeCell(
        const MAP& map,
        int x,
        int y,
        const Unit* self
    );

    bool canMoveToNeighbour(
        const MAP& map,
        const Unit* self,
        Coordinate current,
        int dx,
        int dy
    );
}
