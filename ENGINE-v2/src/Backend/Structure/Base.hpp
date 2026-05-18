#pragma once

#include "Map.hpp"
#include "UnitManager.hpp"

bool place_base(
    MAP& map,
    int x,
    int y,
    int player_id
);

int create_collectors_around_base(
    MAP& map,
    UnitManager& unitManager,
    int base_x,
    int base_y,
    int player_id,
    int quantity
);

bool create_player_base_with_collectors(
    MAP& map,
    UnitManager& unitManager,
    int base_x,
    int base_y,
    int player_id,
    int collector_quantity
);
