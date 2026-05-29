#include "CombatSystem.hpp"

#include "../Building/Building.hpp"

#include <algorithm>
#include <cstdlib>

namespace
{
    int absInt(int value)
    {
        return value < 0 ? -value : value;
    }

    int chebyshevDistance(Coordinate a, Coordinate b)
    {
        int dx = absInt(a.getX() - b.getX());
        int dy = absInt(a.getY() - b.getY());
        return std::max(dx, dy);
    }

    int distanceToBuilding(Coordinate pos, const Building* building)
    {
        if (building == nullptr) {
            return 999999;
        }

        const BuildingDef& def = getBuildingDef(building->getType());

        int minX = building->getMapX();
        int maxX = building->getMapX() + def.sizeX - 1;
        int minY = building->getMapY();
        int maxY = building->getMapY() + def.sizeY - 1;

        int dx = 0;
        int dy = 0;

        if (pos.getX() < minX) {
            dx = minX - pos.getX();
        }
        else if (pos.getX() > maxX) {
            dx = pos.getX() - maxX;
        }

        if (pos.getY() < minY) {
            dy = minY - pos.getY();
        }
        else if (pos.getY() > maxY) {
            dy = pos.getY() - maxY;
        }

        return std::max(dx, dy);
    }

    Unit* findEnemyUnitInRange(
        const Unit* attacker,
        const std::vector<std::unique_ptr<Unit>>& units,
        int range
    ) {
        Unit* bestTarget = nullptr;
        int bestDistance = 0;

        for (const auto& targetPtr : units) {
            Unit* target = targetPtr.get();

            if (target == nullptr || target == attacker || !target->isAlive()) {
                continue;
            }

            if (target->getTeam() == attacker->getTeam()) {
                continue;
            }

            int distance = chebyshevDistance(attacker->getPos(), target->getPos());

            if (distance > range) {
                continue;
            }

            if (bestTarget == nullptr || distance < bestDistance) {
                bestTarget = target;
                bestDistance = distance;
            }
        }

        return bestTarget;
    }

    Building* findEnemyBuildingInRange(
        const Unit* attacker,
        const std::vector<std::unique_ptr<Player>>& players,
        int range
    ) {
        Building* bestTarget = nullptr;
        int bestDistance = 0;

        for (const auto& playerPtr : players) {
            const Player* player = playerPtr.get();

            if (player == nullptr) {
                continue;
            }

            for (const auto& buildingPtr : player->getBuildings()) {
                Building* building = buildingPtr.get();

                if (building == nullptr || !building->isAlive()) {
                    continue;
                }

                if (building->getTeam() == attacker->getTeam()) {
                    continue;
                }

                int distance = distanceToBuilding(attacker->getPos(), building);

                if (distance > range) {
                    continue;
                }

                if (bestTarget == nullptr || distance < bestDistance) {
                    bestTarget = building;
                    bestDistance = distance;
                }
            }
        }

        return bestTarget;
    }

    void removeDeadUnitsFromMap(
        MAP& map,
        const std::vector<std::unique_ptr<Unit>>& units
    ) {
        for (const auto& unitPtr : units) {
            Unit* unit = unitPtr.get();

            if (unit == nullptr || unit->isAlive()) {
                continue;
            }

            int x = unit->getPos().getX();
            int y = unit->getPos().getY();

            if (in_map(map, x, y) && map[x][y].unit == unit) {
                map[x][y].unit = nullptr;
            }
        }
    }
}


void CombatSystem::update(
    MAP& map,
    std::vector<std::unique_ptr<Player>>& players,
    std::vector<std::unique_ptr<Unit>>& units,
    int tickRate,
    Sound* sound
) {
    (void)map;

    for (auto& attackerPtr : units) {
        Unit* attacker = attackerPtr.get();

        if (attacker == nullptr || !attacker->isAlive() || !attacker->canAttack()) {
            continue;
        }

        attacker->tickAttackCooldown();

        if (!attacker->isAttackReady()) {
            continue;
        }

        int range = attacker->getAttackRangeCells();
        int damage = attacker->getAttackDamage();

        Unit* unitTarget = findEnemyUnitInRange(attacker, units, range);

        if (unitTarget != nullptr) {
            unitTarget->takeDamage(damage);
            attacker->resetAttackCooldown(tickRate);
            if (sound) sound->play("attack");
            continue;
        }

        Building* buildingTarget = findEnemyBuildingInRange(attacker, players, range);

        if (buildingTarget != nullptr) {
            buildingTarget->takeDamage(damage);
            attacker->resetAttackCooldown(tickRate);
            if (sound) sound->play("attack");
        }
    }
}

// Par :
bool CombatSystem::removeDeadEntities(
    MAP& map,
    std::vector<std::unique_ptr<Player>>& players,
    std::vector<std::unique_ptr<Unit>>& units,
    Sound* sound
)
 {
    bool removed = false;

    removeDeadUnitsFromMap(map, units);

    auto oldUnitCount = units.size();

    if (sound) {
        for (const auto& unit : units) {
            if (unit && !unit->isAlive()) {
                sound->play("death");
            }
        }
    }

    units.erase(
        std::remove_if(
            units.begin(),
            units.end(),
            [](const std::unique_ptr<Unit>& unit) {
                return unit == nullptr || !unit->isAlive();
            }
        ),
        units.end()
    );

    if (units.size() != oldUnitCount) {
        removed = true;
    }

    for (auto& player : players) {
        if (player && player->removeDeadBuildings(map)) {
            removed = true;
        }
    }

    return removed;
}
