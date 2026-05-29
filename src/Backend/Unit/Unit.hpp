#pragma once

#include "../Entity/Entity.hpp"
#include "../Coordinate/Coordinate.hpp"

#include <memory>
#include <vector>

class Renderer;
class Cell;
class Player;

using MAP = std::vector<std::vector<Cell>>;

class Unit : public Entity
{
public:
    Unit(int id, int team, int x, int y);
    virtual ~Unit();

    void moveTo(int x, int y);

    virtual void update() override;
    void updateDt(float dt);

    /*
     * IA optionnelle.
     * Par défaut, une unité normale utilise seulement cette méthode pour
     * gérer le mode offensif demandé par A + clic gauche.
     */
    virtual void updateAI(
        MAP& map,
        const std::vector<std::unique_ptr<Unit>>& allUnits,
        std::vector<std::unique_ptr<Player>>& players,
        int tickRate
    );

    /*
     * Déplacement manuel actuel.
     * Ce système reste lié aux ordres du joueur et au MassPath.
     */
    void updateMove(
        MAP& map,
        const std::vector<std::unique_ptr<Unit>>& allUnits,
        int tickRate
    );

    virtual void render(Renderer* renderer, int offsetX, int offsetY, int scale) const;

    bool isSelected() const;
    void setSelected(bool sel);

    void setDestination(
        Coordinate dest,
        MAP& map,
        const std::vector<std::unique_ptr<Unit>>& allUnits
    );

    /*
     * Mode offensif.
     * Utilisé par A + clic gauche : l'unité avance vers un point, mais si elle
     * détecte un ennemi pendant le trajet, elle interrompt sa marche et va le frapper.
     */
    void setOffensiveDestination(
        Coordinate dest,
        MAP& map,
        const std::vector<std::unique_ptr<Unit>>& allUnits
    );

    void clearOffensiveMode();
    bool isOffensiveMode() const;

    Coordinate getPos() const
    {
        return position;
    }

    bool hasPath() const
    {
        return !path.empty();
    }

    /*
     * Combat.
     * Une Unit normale représente actuellement le soldat.
     * Les collecteurs redéfinissent canAttack() à false.
     */
    virtual bool canAttack() const;
    virtual int  getAttackDamage() const;
    virtual int  getAttackRangeCells() const;

    void tickAttackCooldown();
    bool isAttackReady() const;
    void resetAttackCooldown(int tickRate);

    static constexpr int RADIUS = 6;

private:
    void stopMovement();
    void forceDestination(Coordinate dest);

    void updateOffensiveAI(
        MAP& map,
        const std::vector<std::unique_ptr<Unit>>& allUnits,
        std::vector<std::unique_ptr<Player>>& players
    );

private:
    bool selected { false };

    std::vector<Coordinate> path;

    Coordinate destination { -1, -1 };

    bool hasTarget { false };

    int ticksWaited { 0 };
    int waitBlocked { 0 };

    int attackCooldownTicks { 0 };

    bool offensiveMode { false };
    Coordinate offensiveGoal { -1, -1 };
    int offensiveThinkTicks { 0 };
};
