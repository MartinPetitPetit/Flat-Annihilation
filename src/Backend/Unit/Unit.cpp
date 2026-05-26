#include "Unit.hpp"
#include "../../Frontend/Renderer/Renderer.hpp"

Unit::Unit(int id, int team, int x, int y)
{
    this->id   = id;
    this->team = team;
    this->position.setX(x);
    this->position.setY(y);
    this->health = 100;
}

Unit::~Unit() {}

void Unit::moveTo(int x, int y)
{
    position.setX(x);
    position.setY(y);
}

void Unit::updateDt(float dt) { (void)dt; }

bool Unit::isSelected()      const { return selected; }
void Unit::setSelected(bool sel)   { selected = sel;  }

// ---------- Destination & pathfinding ----------

void Unit::setDestination(Coordinate dest,
                           MAP& map,
                           const std::vector<std::unique_ptr<Unit>>& allUnits)
{
    destination = dest;
    hasTarget   = true;
    waitBlocked = 0;
    ticksWaited = 0;
    path = findPath(map, position, destination, allUnits, id);
}

// ---------- Mouvement (appelé par Game::update) ----------

void Unit::updateMove(MAP& map,
                      const std::vector<std::unique_ptr<Unit>>& allUnits,
                      int tickRate)
{
    if (!hasTarget || path.empty()) return;

    // 1 case / seconde → on avance toutes les tickRate ticks
    ticksWaited++;
    if (ticksWaited < tickRate) return;
    ticksWaited = 0;

    Coordinate next = path.front();
    int nx = next.getX();
    int ny = next.getY();

    // Vérifier si la prochaine case est libre
    bool blocked = false;

    if (!in_map(map, nx, ny)) {
        blocked = true;
    } else {
        const Cell& c = map[nx][ny];
        if (c.type_terrain == Montain ||
            c.type_terrain == Lake    ||
            c.type_terrain == ravine  ||
            c.buildingID   != -1)
            blocked = true;

        if (c.unit != nullptr && c.unit->getId() != id)
            blocked = true;
    }

    if (blocked) {
        waitBlocked++;
        if (waitBlocked >= 2) {
            // Recalcul du chemin
            waitBlocked = 0;
            path = findPath(map, position, destination, allUnits, id);
        }
        return;
    }

    // --- Déplacement ---
    waitBlocked = 0;

    // Libérer l'ancienne cellule
    int ox = position.getX();
    int oy = position.getY();
    if (in_map(map, ox, oy))
        map[ox][oy].unit = nullptr;

    // Occuper la nouvelle cellule
    map[nx][ny].unit = this;
    position.setX(nx);
    position.setY(ny);

    path.erase(path.begin());

    // Arrivée à destination
    if (path.empty()) {
        hasTarget   = false;
        waitBlocked = 0;
    }
}

// ---------- Rendu ----------

void Unit::render(Renderer* r, int offsetX, int offsetY, int scale) const
{
    int cx = offsetX + position.getX() * scale + scale / 2;
    int cy = offsetY + position.getY() * scale + scale / 2;
    int radius = std::max(3, scale / 2 - 1);

    SDL_Color body = (team == 0)
        ? SDL_Color{ 60, 120, 255, 255 }
        : SDL_Color{ 255,  60,  60, 255 };

    if (selected)
        r->drawFilledCircle(cx, cy, radius + 3, { 255, 220, 0, 180 });

    r->drawFilledCircle(cx, cy, radius, body);

    // Barre de vie
    int barW = radius * 2;
    int barH = 2;
    SDL_Rect barBg = { cx - radius, cy - radius - 4, barW, barH };
    SDL_Rect barFg = { cx - radius, cy - radius - 4, barW * health / 100, barH };
    r->drawRect(barBg, { 80,   0,  0, 200 }, true);
    r->drawRect(barFg, {  0, 220,  0, 220 }, true);
}

void Unit::update() {}