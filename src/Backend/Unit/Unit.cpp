

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

void Unit::updateDt(float dt)
{
    (void)dt;
}

bool Unit::isSelected() const { return selected; }
void Unit::setSelected(bool sel) { selected = sel; }

void Unit::render(Renderer* r, int offsetX, int offsetY, int scale) const
{
    int cx = offsetX + position.getX() * scale + scale / 2;
    int cy = offsetY + position.getY() * scale + scale / 2;
    int radius = std::max(3, scale / 2 - 1);

    // Couleur selon équipe
    SDL_Color body = (team == 0)
        ? SDL_Color{ 60, 120, 255, 255 }   // bleu joueur
        : SDL_Color{ 255,  60,  60, 255 };  // rouge IA

    // Surbrillance si sélectionné
    if (selected) {
        r->drawFilledCirclePublic(cx, cy, radius + 3, { 255, 220, 0, 180 });
    }

    r->drawFilledCirclePublic(cx, cy, radius, body);

    // Barre de vie
    int barW = radius * 2;
    int barH = 2;
    SDL_Rect barBg  = { cx - radius, cy - radius - 4, barW, barH };
    SDL_Rect barFg  = { cx - radius, cy - radius - 4, barW * health / 100, barH };
    r->drawRect(barBg, { 80, 0, 0, 200 }, true);
    r->drawRect(barFg, { 0, 220, 0, 220 }, true);
}
void Unit::update()
{
    // appelé par le moteur sans dt
}