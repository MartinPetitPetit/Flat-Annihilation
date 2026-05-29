#include "Collector.hpp"

#include "../Map/Map.hpp"
#include "../Pathing/AIPath.hpp"
#include "../Pathing/MovementRules.hpp"
#include "../Player/Player.hpp"
#include "../../Frontend/Renderer/Renderer.hpp"

#include <algorithm>
#include <array>
#include <unordered_map>

namespace
{
    constexpr int COLLECTOR_MOVES_PER_SECOND = 5;
    constexpr int GATHER_AMOUNT_PER_ACTION = 5;
    constexpr int MAX_PATH_CANDIDATES_PER_SEARCH = 8;
    constexpr int PATH_MAX_VISITED_NODES = 5000;
    constexpr int MAX_RESERVATIONS_PER_RESOURCE = 2;
    constexpr int MAX_PATH_FAILS_BEFORE_ABANDON = 3;

    /*
     * Recherche progressive.
     * Le collecteur commence près de lui, puis agrandit le rayon.
     * Cela évite de scanner toute la carte dès le premier essai.
     */
    constexpr std::array<int, 7> SEARCH_RADII {{
        8, 16, 32, 64, 96, 128, 192
    }};

    struct Candidate
    {
        Coordinate pos;
        int dist;
    };

    std::unordered_map<long long, int> resourceReservations;
    std::unordered_map<const Collector*, long long> collectorReservation;

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

    bool sameCell(Coordinate a, Coordinate b)
    {
        return a.getX() == b.getX() && a.getY() == b.getY();
    }

    long long resourceKey(Coordinate pos)
    {
        return (static_cast<long long>(pos.getX()) << 32) ^
               static_cast<unsigned int>(pos.getY());
    }

    void releaseReservationFor(const Collector* collector)
    {
        auto it = collectorReservation.find(collector);

        if (it == collectorReservation.end()) {
            return;
        }

        long long key = it->second;
        collectorReservation.erase(it);

        auto reservationIt = resourceReservations.find(key);

        if (reservationIt == resourceReservations.end()) {
            return;
        }

        reservationIt->second--;

        if (reservationIt->second <= 0) {
            resourceReservations.erase(reservationIt);
        }
    }

    bool canReserveResourceFor(const Collector* collector, Coordinate pos)
    {
        long long key = resourceKey(pos);

        auto ownIt = collectorReservation.find(collector);

        if (ownIt != collectorReservation.end() && ownIt->second == key) {
            return true;
        }

        auto reservationIt = resourceReservations.find(key);

        if (reservationIt == resourceReservations.end()) {
            return true;
        }

        return reservationIt->second < MAX_RESERVATIONS_PER_RESOURCE;
    }

    void reserveResourceFor(const Collector* collector, Coordinate pos)
    {
        releaseReservationFor(collector);

        long long key = resourceKey(pos);
        collectorReservation[collector] = key;
        resourceReservations[key]++;
    }

    bool isResourceInteresting(Resource* resource, ResourceType wanted)
    {
        return resource != nullptr &&
               !resource->isEmpty() &&
               resource->getResourceType() == wanted;
    }

    bool canUseAsDepositCell(const MAP& map, int x, int y, const Unit* self)
    {
        if (!MovementRules::isFreeCell(map, x, y, self)) {
            return false;
        }

        if (map[x][y].resource != nullptr) {
            return false;
        }

        return true;
    }

    bool findNearestFreeCellAroundBuilding(
        const MAP& map,
        const Building* building,
        const Unit* self,
        Coordinate start,
        Coordinate& outCell
    ) {
        if (building == nullptr) {
            return false;
        }

        const BuildingDef& def = getBuildingDef(building->getType());

        int bx = building->getMapX();
        int by = building->getMapY();

        bool found = false;
        int bestDistance = 0;
        Coordinate best(0, 0);

        for (int radius = 1; radius <= 8; radius++) {
            int minX = bx - radius;
            int maxX = bx + def.sizeX - 1 + radius;
            int minY = by - radius;
            int maxY = by + def.sizeY - 1 + radius;

            for (int x = minX; x <= maxX; x++) {
                for (int y = minY; y <= maxY; y++) {
                    bool border =
                        x == minX ||
                        x == maxX ||
                        y == minY ||
                        y == maxY;

                    if (!border) {
                        continue;
                    }

                    if (!in_map(map, x, y)) {
                        continue;
                    }

                    if (!canUseAsDepositCell(map, x, y, self)) {
                        continue;
                    }

                    Coordinate candidate(x, y);
                    int d = distance2(start, candidate);

                    if (!found || d < bestDistance) {
                        found = true;
                        bestDistance = d;
                        best = candidate;
                    }
                }
            }

            if (found) {
                outCell = best;
                return true;
            }
        }

        return false;
    }
}

Collector::Collector(int id, int team, int x, int y)
    : Unit(id, team, x, y)
{
    maxHealth = 70;
    health = maxHealth;

    /*
     * Décalage initial.
     * Si 20 collecteurs naissent en même temps, ils ne lancent pas tous
     * la recherche de ressource exactement au même tick.
     */
    searchCooldownTicks = id % 10;
}

Collector::~Collector()
{
    releaseReservationFor(this);
}

void Collector::updateCooldowns()
{
    if (searchCooldownTicks > 0) {
        searchCooldownTicks--;
    }

    if (repathCooldownTicks > 0) {
        repathCooldownTicks--;
    }
}

void Collector::setSearchCooldown(int ticks)
{
    if (ticks > searchCooldownTicks) {
        searchCooldownTicks = ticks;
    }
}

void Collector::setRepathCooldown(int tickRate)
{
    repathCooldownTicks = std::max(2, tickRate / 2 + (getId() % 5));
}

bool Collector::isAdjacentTo(Coordinate a, Coordinate b) const
{
    int dx = absInt(a.getX() - b.getX());
    int dy = absInt(a.getY() - b.getY());

    return dx <= 1 && dy <= 1;
}

bool Collector::isTargetResourceValid(const MAP& map) const
{
    if (!hasTargetResource) {
        return false;
    }

    int x = targetResourcePos.getX();
    int y = targetResourcePos.getY();

    if (!in_map(map, x, y)) {
        return false;
    }

    return isResourceInteresting(map[x][y].resource, targetResourceType);
}

void Collector::clearPath()
{
    aiPath.clear();
    aiPathIndex = 0;
    pathTicks = 0;
}

void Collector::releaseTargetReservation()
{
    releaseReservationFor(this);
}

void Collector::goSearch(int delayTicks)
{
    releaseTargetReservation();
    hasTargetResource = false;
    hasDepositCell = false;
    pathFailCount = 0;
    clearPath();
    setSearchCooldown(delayTicks);
    state = State::SearchingResource;
}

void Collector::goDeposit()
{
    releaseTargetReservation();
    hasDepositCell = false;
    pathFailCount = 0;
    clearPath();
    state = State::MovingToTownCenter;
}

bool Collector::selectResourceOfTypeInRadius(MAP& map, ResourceType type, int radius)
{
    if (map.empty() || map[0].empty()) {
        return false;
    }

    std::vector<Candidate> candidates;
    Coordinate current = getPos();

    int minX = std::max(0, current.getX() - radius);
    int maxX = std::min(static_cast<int>(map.size()) - 1, current.getX() + radius);
    int minY = std::max(0, current.getY() - radius);
    int maxY = std::min(static_cast<int>(map[0].size()) - 1, current.getY() + radius);
    int radius2 = radius * radius;

    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            Resource* resource = map[x][y].resource;

            if (!isResourceInteresting(resource, type)) {
                continue;
            }

            Coordinate pos(x, y);
            int dist = distance2(current, pos);

            if (dist > radius2) {
                continue;
            }

            if (!canReserveResourceFor(this, pos)) {
                continue;
            }

            candidates.push_back(Candidate{ pos, dist });
        }
    }

    if (candidates.empty()) {
        return false;
    }

    std::sort(
        candidates.begin(),
        candidates.end(),
        [](const Candidate& a, const Candidate& b) {
            return a.dist < b.dist;
        }
    );

    int testedCandidates = 0;

    for (const Candidate& candidate : candidates) {
        if (testedCandidates >= MAX_PATH_CANDIDATES_PER_SEARCH) {
            break;
        }

        testedCandidates++;

        if (isAdjacentTo(current, candidate.pos)) {
            reserveResourceFor(this, candidate.pos);
            targetResourcePos = candidate.pos;
            targetResourceType = type;
            hasTargetResource = true;
            pathFailCount = 0;
            clearPath();
            return true;
        }

        std::vector<Coordinate> path = AIPath::findPathOrEmpty(
            map,
            current,
            candidate.pos,
            this,
            AIPath::GoalMode::AdjacentToGoal,
            PATH_MAX_VISITED_NODES
        );

        if (!path.empty()) {
            reserveResourceFor(this, candidate.pos);
            targetResourcePos = candidate.pos;
            targetResourceType = type;
            hasTargetResource = true;
            aiPath = path;
            aiPathIndex = 0;
            pathTicks = 0;
            pathFailCount = 0;
            return true;
        }
    }

    return false;
}

bool Collector::selectResourceOfType(MAP& map, ResourceType type)
{
    int maxMapDimension = 0;

    if (!map.empty() && !map[0].empty()) {
        maxMapDimension = std::max(
            static_cast<int>(map.size()),
            static_cast<int>(map[0].size())
        );
    }

    int lastRadius = 0;

    for (int radius : SEARCH_RADII) {
        lastRadius = radius;

        if (selectResourceOfTypeInRadius(map, type, radius)) {
            return true;
        }

        if (maxMapDimension > 0 && radius >= maxMapDimension) {
            return false;
        }
    }

    /*
     * Fallback rare pour les grandes cartes.
     * Il n'est appelé qu'après les petits rayons et avec cooldown,
     * donc il évite le blocage où aucun resource proche n'existe.
     */
    if (maxMapDimension > lastRadius) {
        return selectResourceOfTypeInRadius(map, type, maxMapDimension);
    }

    return false;
}

bool Collector::selectNextResource(MAP& map)
{
    if (selectResourceOfType(map, preferredType)) {
        return true;
    }

    ResourceType otherType = preferredType == food ? wood : food;

    if (selectResourceOfType(map, otherType)) {
        return true;
    }

    return false;
}

bool Collector::findDepositCell(MAP& map, std::vector<std::unique_ptr<Player>>& players)
{
    int team = getTeam();

    if (team < 0 || team >= static_cast<int>(players.size()) || !players[team]) {
        return false;
    }

    Coordinate current = getPos();
    Building* bestTownCenter = nullptr;
    int bestDist = 0;
    bool foundTownCenter = false;

    for (const auto& buildingPtr : players[team]->getBuildings()) {
        Building* building = buildingPtr.get();

        if (building == nullptr || !building->isAlive()) {
            continue;
        }

        if (building->getType() != BuildingType::TownCenter) {
            continue;
        }

        Coordinate buildingPos(building->getMapX(), building->getMapY());
        int dist = distance2(current, buildingPos);

        if (!foundTownCenter || dist < bestDist) {
            foundTownCenter = true;
            bestDist = dist;
            bestTownCenter = building;
        }
    }

    if (!foundTownCenter) {
        return false;
    }

    Coordinate cell(0, 0);

    if (!findNearestFreeCellAroundBuilding(map, bestTownCenter, this, current, cell)) {
        return false;
    }

    depositCell = cell;
    hasDepositCell = true;

    if (sameCell(current, depositCell)) {
        clearPath();
        return true;
    }

    aiPath = AIPath::findPathOrEmpty(
        map,
        current,
        depositCell,
        this,
        AIPath::GoalMode::ExactCell,
        PATH_MAX_VISITED_NODES
    );

    aiPathIndex = 0;
    pathTicks = 0;

    if (aiPath.empty()) {
        hasDepositCell = false;
        return false;
    }

    return true;
}

bool Collector::followCurrentPath(MAP& map, int tickRate)
{
    return AIPath::moveOneStepAlongPath(
        map,
        this,
        aiPath,
        aiPathIndex,
        pathTicks,
        tickRate,
        COLLECTOR_MOVES_PER_SECOND
    );
}

void Collector::depositCarriedResource(std::vector<std::unique_ptr<Player>>& players)
{
    int team = getTeam();

    if (team < 0 || team >= static_cast<int>(players.size()) || !players[team]) {
        return;
    }

    if (carriedAmount <= 0) {
        return;
    }

    if (carriedType == wood) {
        players[team]->addWood(carriedAmount);
    }
    else if (carriedType == food) {
        players[team]->addFood(carriedAmount);
    }

    carriedAmount = 0;
    preferredType = preferredType == food ? wood : food;
}

bool Collector::canAttack() const
{
    return false;
}

int Collector::getAttackDamage() const
{
    return 0;
}

int Collector::getAttackRangeCells() const
{
    return 0;
}

void Collector::updateAI(
    MAP& map,
    const std::vector<std::unique_ptr<Unit>>& allUnits,
    std::vector<std::unique_ptr<Player>>& players,
    int tickRate
) {
    (void)allUnits;

    if (!isAlive()) {
        return;
    }

    updateCooldowns();

    if (carriedAmount > 0 && state != State::MovingToTownCenter && state != State::Depositing) {
        if (!isTargetResourceValid(map) || carriedAmount >= capacity) {
            goDeposit();
        }
    }

    switch (state) {
    case State::SearchingResource:
        if (carriedAmount > 0) {
            goDeposit();
            break;
        }

        if (searchCooldownTicks > 0) {
            break;
        }

        if (selectNextResource(map)) {
            state = State::MovingToResource;
        }
        else {
            setSearchCooldown(std::max(5, tickRate + (getId() % 10)));
            state = State::Idle;
        }
        break;

    case State::MovingToResource:
        if (!isTargetResourceValid(map)) {
            if (carriedAmount > 0) {
                goDeposit();
            }
            else {
                goSearch(std::max(3, tickRate / 2 + (getId() % 6)));
            }
            break;
        }

        if (isAdjacentTo(getPos(), targetResourcePos)) {
            clearPath();
            pathFailCount = 0;
            state = State::Gathering;
            break;
        }

        if (aiPath.empty()) {
            if (repathCooldownTicks > 0) {
                break;
            }

            aiPath = AIPath::findPathOrEmpty(
                map,
                getPos(),
                targetResourcePos,
                this,
                AIPath::GoalMode::AdjacentToGoal,
                PATH_MAX_VISITED_NODES
            );

            aiPathIndex = 0;
            pathTicks = 0;

            if (aiPath.empty()) {
                pathFailCount++;
                setRepathCooldown(tickRate);

                if (pathFailCount >= MAX_PATH_FAILS_BEFORE_ABANDON) {
                    goSearch(std::max(5, tickRate + (getId() % 8)));
                }

                break;
            }

            pathFailCount = 0;
        }

        if (!followCurrentPath(map, tickRate) && !isAdjacentTo(getPos(), targetResourcePos)) {
            clearPath();
            pathFailCount++;
            setRepathCooldown(tickRate);

            if (pathFailCount >= MAX_PATH_FAILS_BEFORE_ABANDON) {
                goSearch(std::max(5, tickRate + (getId() % 8)));
            }
        }
        break;

    case State::Gathering:
        if (!isTargetResourceValid(map)) {
            if (carriedAmount > 0) {
                goDeposit();
            }
            else {
                goSearch(std::max(3, tickRate / 2 + (getId() % 6)));
            }
            break;
        }

        if (!isAdjacentTo(getPos(), targetResourcePos)) {
            state = State::MovingToResource;
            break;
        }

        gatherTicks++;

        if (gatherTicks < std::max(1, tickRate / 2)) {
            break;
        }

        gatherTicks = 0;

        {
            Resource* resource = map[targetResourcePos.getX()][targetResourcePos.getY()].resource;
            int freeSpace = capacity - carriedAmount;
            int taken = resource->gather(std::min(GATHER_AMOUNT_PER_ACTION, freeSpace));

            if (taken > 0) {
                carriedType = targetResourceType;
                carriedAmount += taken;
            }

            if (resource->isEmpty()) {
                delete resource;
                map[targetResourcePos.getX()][targetResourcePos.getY()].resource = nullptr;
                hasTargetResource = false;
                releaseTargetReservation();
            }
        }

        if (carriedAmount >= capacity || !hasTargetResource) {
            if (carriedAmount > 0) {
                goDeposit();
            }
            else {
                goSearch(std::max(3, tickRate / 2 + (getId() % 6)));
            }
        }
        break;

    case State::MovingToTownCenter:
        if (carriedAmount <= 0) {
            goSearch(std::max(2, getId() % 5));
            break;
        }

        if (sameCell(getPos(), depositCell)) {
            state = State::Depositing;
            break;
        }

        if (!hasDepositCell || aiPath.empty()) {
            if (repathCooldownTicks > 0) {
                break;
            }

            if (!findDepositCell(map, players)) {
                pathFailCount++;
                setRepathCooldown(tickRate);

                if (pathFailCount >= MAX_PATH_FAILS_BEFORE_ABANDON) {
                    state = State::Idle;
                    setSearchCooldown(std::max(5, tickRate));
                }
                break;
            }

            pathFailCount = 0;
        }

        if (!followCurrentPath(map, tickRate) && !sameCell(getPos(), depositCell)) {
            hasDepositCell = false;
            clearPath();
            pathFailCount++;
            setRepathCooldown(tickRate);
        }
        break;

    case State::Depositing:
        depositCarriedResource(players);
        goSearch(std::max(2, getId() % 5));
        break;

    case State::Idle:
        if (carriedAmount > 0) {
            goDeposit();
        }
        else {
            if (searchCooldownTicks > 0) {
                break;
            }

            if (selectNextResource(map)) {
                state = State::MovingToResource;
            }
            else {
                setSearchCooldown(std::max(5, tickRate + (getId() % 10)));
            }
        }
        break;
    }
}

void Collector::render(Renderer* r, int offsetX, int offsetY, int scale) const
{
    int cx = offsetX + getPos().getX() * scale + scale / 2;
    int cy = offsetY + getPos().getY() * scale + scale / 2;
    int radius = std::max(3, scale / 2 - 1);

    if (isSelected()) {
        r->drawFilledCircle(cx, cy, radius + 3, {255, 220, 0, 180});
    }

    SDL_Color body = (getTeam() == 0)
        ? SDL_Color{40, 120, 255, 255}
        : SDL_Color{220, 60, 60, 255};

    r->drawFilledCircle(cx, cy, radius, body);

    int crossThickness = std::max(1, scale / 8);
    int crossLength = std::max(4, radius);

    SDL_Rect horizontal = {
        cx - crossLength,
        cy - crossThickness / 2,
        crossLength * 2,
        crossThickness
    };

    SDL_Rect vertical = {
        cx - crossThickness / 2,
        cy - crossLength,
        crossThickness,
        crossLength * 2
    };

    r->drawRect(horizontal, {230, 245, 255, 255}, true);
    r->drawRect(vertical,   {230, 245, 255, 255}, true);

    int barW = radius * 2;
    int barH = 2;

    SDL_Rect barBg = {
        cx - radius,
        cy - radius - 4,
        barW,
        barH
    };

    int maxHp = std::max(1, getMaxHealth());

    SDL_Rect barFg = {
        cx - radius,
        cy - radius - 4,
        barW * getHealth() / maxHp,
        barH
    };

    r->drawRect(barBg, {80, 0, 0, 200}, true);
    r->drawRect(barFg, {0, 220, 0, 220}, true);
}
