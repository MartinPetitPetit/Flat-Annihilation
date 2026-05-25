#pragma once

#include "../Player/Player.hpp"
#include "../Entity/Entity.hpp"
#include "../Map/Map.hpp"
#include "../Resource/Resource.hpp"

class Renderer;

class Unit : public Entity
{
public:
    Unit(int id, int team, int x, int y);
    virtual ~Unit();

    void moveTo(int x, int y);
    virtual void update() override;
	void update(float dt);	
	void updateDt(float dt);
    void render(Renderer* renderer, int offsetX, int offsetY, int scale) const;

    bool isSelected() const;
    void setSelected(bool sel);

    // Rayon visuel en cellules (demi-taille du sprite)
    static constexpr int RADIUS = 6; // pixels à l'échelle 1

private:
    bool selected { false };
};