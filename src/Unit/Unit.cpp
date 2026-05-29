#include "Unit.hpp"

#include "../Map/Map.hpp"
#include "../Pathing/MassPath.hpp"
#include "../Pathing/MovementRules.hpp"
#include "../../Frontend/Renderer/Renderer.hpp"

#include <algorithm>
#include <array>
#include <limits>

namespace
{
    int distance2(Coordinate a, Coordinate b)
    {
        int dx = a.getX() - b.getX();
        int dy = a.getY() - b.getY();

        return dx * dx + dy * dy;
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
    /*
     * Unité normale : aucune IA.
     * Collector et Enemy redéfinissent cette fonction.
     */
    (void)map;
    (void)allUnits;
    (void)players;
    (void)tickRate;
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

    destination = dest;
    hasTarget = true;
    ticksWaited = 0;
    waitBlocked = 0;
    path.clear();
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
