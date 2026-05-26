#pragma once

#include "../Player/Player.hpp"
#include "../Entity/Entity.hpp"
#include "../Map/Map.hpp"
#include "../Resource/Resource.hpp"
#include <vector>
#include <memory>

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
    void updateMove(MAP& map,
                    const std::vector<std::unique_ptr<Unit>>& allUnits,
                    int tickRate);

    void render(Renderer* renderer, int offsetX, int offsetY, int scale) const;

    bool isSelected() const;
    void setSelected(bool sel);

    void setDestination(Coordinate dest,
                        MAP& map,
                        const std::vector<std::unique_ptr<Unit>>& allUnits);

    bool hasPath() const { return !path.empty(); }

    static constexpr int RADIUS = 6;

private:
    bool selected { false };

    std::vector<Coordinate> path;
    Coordinate              destination   { -1, -1 };
    bool                    hasTarget     { false   };
    int                     ticksWaited   { 0       }; // compteur depuis dernier déplacement
    int                     waitBlocked   { 0       }; // compteur de ticks bloqués
};