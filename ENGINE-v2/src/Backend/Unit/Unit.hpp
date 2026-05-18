#pragma once

#include <vector>

#include "Cell.hpp"

struct UnitBase {
    UNIT type;

    int x;
    int y;

    int hp;
    int max_hp;

    float movement_speed;

    /*
     * Owner player id.
     * 0 = first player
     * 1 = second player
     * 2 = third player / first AI, etc.
     */
    int owner_id;

    std::vector<TERRAIN> allowed_terrains;

    bool can_walk_on(TERRAIN terrain) const
    {
        for (TERRAIN allowed : allowed_terrains)
        {
            if (allowed == terrain)
                return true;
        }

        return false;
    }
};
