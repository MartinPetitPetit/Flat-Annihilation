#include "Unit.hpp"

#include "../Building/Building.hpp"
#include "../Map/Map.hpp"
#include "../Pathing/MassPath.hpp"
#include "../Pathing/MovementRules.hpp"
#include "../Player/Player.hpp"
#include "../../Frontend/Renderer/Renderer.hpp"

#include <algorithm>
#include <array>
#include <limits>

namespace
{
    constexpr int OFFENSIVE_AGGRO_RANGE = 7;
    constexpr int OFFENSIVE_THINK_DELAY_TICKS = 2;

    int absInt(int value)
    {
        return value < 0 ? -value : value;
    }

    int distance2(Coordinate a, Coordinate b)
    {
        int dx = a.getX() - b.getX();
        int dy = a.getY() - b.getY();

        return dx * dx + dy * dy;
    }

    int chebyshevDistance(Coordinate a, Coordinate b)
    {
        int dx = absInt(a.getX() - b.getX());
        int dy = absInt(a.getY() - b.getY());
        return std::max(dx, dy);
    }

    bool sameCell(Coordinate a, Coordinate b)
    {
        return a.getX() == b.getX() && a.getY() == b.getY();
    }

    Coordinate clampToBuilding(Coordinate pos, const Building* building)
    {
        const BuildingDef& def = getBuildingDef(building->getType());

        int minX = building->getMapX();
        int maxX = building->getMapX() + def.sizeX - 1;
        int minY = building->getMapY();
        int maxY = building->getMapY() + def.sizeY - 1;

        int x = std::max(minX, std::min(maxX, pos.getX()));
        int y = std::max(minY, std::min(maxY, pos.getY()));

        return Coordinate(x, y);
    }

    int distanceToBuilding(Coordinate pos, const Building* building)
    {
        if (building == nullptr) {
            return std::numeric_limits<int>::max();
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

    Unit* findClosestEnemyUnit(
        const Unit* self,
        const std::vector<std::unique_ptr<Unit>>& allUnits,
        int range
    ) {
        Unit* best = nullptr;
        int bestDistance = std::numeric_limits<int>::max();

        for (const auto& unitPtr : allUnits) {
            Unit* candidate = unitPtr.get();

            if (candidate == nullptr || candidate == self || !candidate->isAlive()) {
                continue;
            }

            if (candidate->getTeam() == self->getTeam()) {
                continue;
            }

            int distance = chebyshevDistance(self->getPos(), candidate->getPos());

            if (distance > range) {
                continue;
            }

            if (best == nullptr || distance < bestDistance) {
                best = candidate;
                bestDistance = distance;
            }
        }

        return best;
    }

    Building* findClosestEnemyBuilding(
        const Unit* self,
        const std::vector<std::unique_ptr<Player>>& players,
        int range
    ) {
        Building* best = nullptr;
        int bestDistance = std::numeric_limits<int>::max();

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

                if (building->getTeam() == self->getTeam()) {
                    continue;
                }

                int distance = distanceToBuilding(self->getPos(), building);

                if (distance > range) {
                    continue;
                }

                if (best == nullptr || distance < bestDistance) {
                    best = building;
                    bestDistance = distance;
                }
            }
        }

        return best;
    }

    bool chooseNeighbour(
        const MAP& map,
        const Unit* self,
        Coordinate current,
        Coordinate target,
        Coordinate& next
    ) {
        static const std::array<std::array<int, 2>, 8> directions {{
            {{ 1,  0}},
            {{-1,  0}},
            {{ 0,  1}},
            {{ 0, -1}},
            {{ 1,  1}},
            {{ 1, -1}},
            {{-1,  1}},
            {{-1, -1}}
        }};

        int currentDistance = distance2(current, target);

        bool found = false;
        int bestDistance = std::numeric_limits<int>::max();

        for (const auto& direction : directions) {
            int dx = direction[0];
            int dy = direction[1];

            if (!MovementRules::canMoveToNeighbour(map, self, current, dx, dy)) {
                continue;
            }

            Coordinate candidate(
                current.getX() + dx,
                current.getY() + dy
            );

            int candidateDistance = distance2(candidate, target);

            if (candidateDistance >= currentDistance) {
                continue;
            }

            if (!found || candidateDistance < bestDistance) {
                found = true;
                bestDistance = candidateDistance;
                next = candidate;
            }
        }

        return found;
    }
}

Unit::Unit(int id, int team, int x, int y)
{
    this->id = id;
    this->team = team;

    position.setX(x);
    position.setY(y);

    maxHealth = 100;
    health = maxHealth;
}

Unit::~Unit()
{
}

void Unit::moveTo(int x, int y)
{
    position.setX(x);
    position.setY(y);
}

void Unit::update()
{
}

void Unit::updateDt(float dt)
{
    (void)dt;
}

void Unit::updateAI(
    MAP& map,
    const std::vector<std::unique_ptr<Unit>>& allUnits,
    std::vector<std::unique_ptr<Player>>& players,
    int tickRate
) {
    (void)tickRate;

    if (!isAlive()) {
        return;
    }

    updateOffensiveAI(map, allUnits, players);
}

void Unit::updateOffensiveAI(
    MAP& map,
    const std::vector<std::unique_ptr<Unit>>& allUnits,
    std::vector<std::unique_ptr<Player>>& players
) {
    (void)map;

    if (!offensiveMode || !canAttack()) {
        return;
    }

    offensiveThinkTicks++;

    if (offensiveThinkTicks < OFFENSIVE_THINK_DELAY_TICKS) {
        return;
    }

    offensiveThinkTicks = 0;

    int attackRange = getAttackRangeCells();

    Unit* enemyUnit = findClosestEnemyUnit(this, allUnits, OFFENSIVE_AGGRO_RANGE);

    if (enemyUnit != nullptr) {
        int distance = chebyshevDistance(getPos(), enemyUnit->getPos());

        if (distance <= attackRange) {
            stopMovement();
        }
        else {
            forceDestination(enemyUnit->getPos());
        }

        return;
    }

    Building* enemyBuilding = findClosestEnemyBuilding(this, players, OFFENSIVE_AGGRO_RANGE);

    if (enemyBuilding != nullptr) {
        int distance = distanceToBuilding(getPos(), enemyBuilding);

        if (distance <= attackRange) {
            stopMovement();
        }
        else {
            forceDestination(clampToBuilding(getPos(), enemyBuilding));
        }

        return;
    }

    if (!sameCell(getPos(), offensiveGoal)) {
        forceDestination(offensiveGoal);
    }
    else {
        stopMovement();
    }
}

bool Unit::canAttack() const
{
    return isAlive();
}

int Unit::getAttackDamage() const
{
    return 10;
}

int Unit::getAttackRangeCells() const
{
    return 1;
}

void Unit::tickAttackCooldown()
{
    if (attackCooldownTicks > 0) {
        attackCooldownTicks--;
    }
}

bool Unit::isAttackReady() const
{
    return attackCooldownTicks <= 0;
}

void Unit::resetAttackCooldown(int tickRate)
{
    attackCooldownTicks = std::max(1, tickRate);
}

bool Unit::isSelected() const
{
    return selected;
}

void Unit::setSelected(bool sel)
{
    selected = sel;
}

void Unit::setDestination(
    Coordinate dest,
    MAP& map,
    const std::vector<std::unique_ptr<Unit>>& allUnits
) {
    (void)map;
    (void)allUnits;

    clearOffensiveMode();
    forceDestination(dest);
}

void Unit::setOffensiveDestination(
    Coordinate dest,
    MAP& map,
    const std::vector<std::unique_ptr<Unit>>& allUnits
) {
    (void)map;
    (void)allUnits;

    if (!canAttack()) {
        return;
    }

    MassPath::clearPlan(this);

    offensiveMode = true;
    offensiveGoal = dest;
    offensiveThinkTicks = 0;

    forceDestination(dest);
}

void Unit::clearOffensiveMode()
{
    offensiveMode = false;
    offensiveGoal = Coordinate(-1, -1);
    offensiveThinkTicks = 0;
}

bool Unit::isOffensiveMode() const
{
    return offensiveMode;
}

void Unit::stopMovement()
{
    hasTarget = false;
    waitBlocked = 0;
    ticksWaited = 0;
    path.clear();
    MassPath::clearPlan(this);
}

void Unit::forceDestination(Coordinate dest)
{
    destination = dest;
    hasTarget = true;
    ticksWaited = 0;
    waitBlocked = 0;
    path.clear();
    MassPath::clearPlan(this);
}

void Unit::updateMove(
    MAP& map,
    const std::vector<std::unique_ptr<Unit>>& allUnits,
    int tickRate
) {
    (void)allUnits;

    if (!isAlive()) {
        return;
    }

    Coordinate target = destination;

    if (MassPath::hasPlan(this)) {
        if (!MassPath::syncPlanWithUnit(map, this, position, target)) {
            hasTarget = false;
            waitBlocked = 0;
            return;
        }

        destination = target;
        hasTarget = true;
    }

    if (!hasTarget) {
        return;
    }

    if (position.getX() == destination.getX() &&
        position.getY() == destination.getY()) {
        hasTarget = false;
        waitBlocked = 0;
        MassPath::clearPlan(this);
        return;
    }

    ticksWaited++;

    int moveDelay = std::max(1, tickRate / 5);

    if (ticksWaited < moveDelay) {
        return;
    }

    ticksWaited = 0;

    Coordinate next;

    if (!chooseNeighbour(map, this, position, destination, next)) {
        hasTarget = false;
        waitBlocked = 0;
        MassPath::clearPlan(this);
        return;
    }

    int oldX = position.getX();
    int oldY = position.getY();

    int newX = next.getX();
    int newY = next.getY();

    if (in_map(map, oldX, oldY) && map[oldX][oldY].unit == this) {
        map[oldX][oldY].unit = nullptr;
    }

    map[newX][newY].unit = this;

    position.setX(newX);
    position.setY(newY);

    waitBlocked = 0;

    if (position.getX() == destination.getX() &&
        position.getY() == destination.getY()) {
        hasTarget = false;
        MassPath::clearPlan(this);
    }
}

void Unit::render(Renderer* r, int offsetX, int offsetY, int scale) const
{
    int cx = offsetX + position.getX() * scale + scale / 2;
    int cy = offsetY + position.getY() * scale + scale / 2;
    int radius = std::max(3, scale / 2 - 1);

    SDL_Color body = (team == 0)
        ? SDL_Color{60, 120, 255, 255}
        : SDL_Color{255, 60, 60, 255};

    if (selected) {
        r->drawFilledCircle(cx, cy, radius + 3, {255, 220, 0, 180});
    }

    r->drawFilledCircle(cx, cy, radius, body);

    if (offensiveMode && canAttack()) {
        int crossSize = std::max(3, radius / 2);
        SDL_SetRenderDrawBlendMode(r->getSDLRenderer(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r->getSDLRenderer(), 255, 255, 255, 230);
        SDL_RenderDrawLine(r->getSDLRenderer(), cx - crossSize, cy, cx + crossSize, cy);
        SDL_RenderDrawLine(r->getSDLRenderer(), cx, cy - crossSize, cx, cy + crossSize);
    }

    int barW = radius * 2;
    int barH = 2;
    int maxHp = std::max(1, getMaxHealth());

    SDL_Rect barBg = {
        cx - radius,
        cy - radius - 4,
        barW,
        barH
    };

    SDL_Rect barFg = {
        cx - radius,
        cy - radius - 4,
        barW * health / maxHp,
        barH
    };

    r->drawRect(barBg, {80, 0, 0, 200}, true);
    r->drawRect(barFg, {0, 220, 0, 220}, true);
}
