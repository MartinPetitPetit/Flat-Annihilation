#pragma once

#include "../Entity/Entity.hpp"
#include "../Coordinate/Coordinate.hpp"

#include <memory>
#include <vector>

class Renderer;
class Cell;

using MAP = std::vector<std::vector<Cell>>;

class Unit : public Entity
{
public:
    Unit(int id, int team, int x, int y);
    virtual ~Unit();

    void moveTo(int x, int y);

    virtual void update() override;
    void updateDt(float dt);

    void updateMove(
        MAP& map,
        const std::vector<std::unique_ptr<Unit>>& allUnits,
        int tickRate
    );

    void render(Renderer* renderer, int offsetX, int offsetY, int scale) const;

    bool isSelected() const;
    void setSelected(bool sel);

    void setDestination(
        Coordinate dest,
        MAP& map,
        const std::vector<std::unique_ptr<Unit>>& allUnits
    );

    /*
     * Accès utilisé par la sélection et par les autres systèmes.
     */
    Coordinate getPos() const
    {
        return position;
    }

    bool hasPath() const
    {
        return !path.empty();
    }

    static constexpr int RADIUS = 6;

private:
    bool selected { false };

    std::vector<Coordinate> path;

    Coordinate destination { -1, -1 };

    bool hasTarget { false };

    int ticksWaited { 0 };
    int waitBlocked { 0 };
};
