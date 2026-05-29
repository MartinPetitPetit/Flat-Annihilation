#pragma once

#include <memory>
#include <vector>

#include "../Map/Map.hpp"
#include "../Player/Player.hpp"
#include "../Unit/Unit.hpp"

class CombatSystem
{
public:
    static void update(
        MAP& map,
        std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<Unit>>& units,
        int tickRate
    );

    static bool removeDeadEntities(
        MAP& map,
        std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<Unit>>& units
    );
};
