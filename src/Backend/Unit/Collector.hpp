/*
 * Backend/Unit/Collector.hpp
 *
 * Rôle du fichier :
 * Declares the Collector unit class, resource state machine fields, and helper methods.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Unit. Il décrit les unités mobiles, leur IA, leur combat, leur déplacement et leur rendu.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#pragma once

#include "Unit.hpp"
#include "../Resource/Resource.hpp"

#include <cstddef>
#include <memory>
#include <vector>

class Player;
class Renderer;

class Collector : public Unit
{
public:
    Collector(int id, int team, int x, int y);
    ~Collector() override;

    void updateAI(
        MAP& map,
        const std::vector<std::unique_ptr<Unit>>& allUnits,
        std::vector<std::unique_ptr<Player>>& players,
        int tickRate
    ) override;

    void render(Renderer* renderer, int offsetX, int offsetY, int scale) const override;

    bool canAttack() const override;
    int  getAttackDamage() const override;
    int  getAttackRangeCells() const override;

private:
    enum class State
    {
        SearchingResource,
        MovingToResource,
        Gathering,
        MovingToTownCenter,
        Depositing,
        Idle
    };

    State state { State::SearchingResource };

    ResourceType preferredType { food };
    ResourceType targetResourceType { food };
    ResourceType carriedType { food };

    Coordinate targetResourcePos { -1, -1 };
    Coordinate depositCell { -1, -1 };

    bool hasTargetResource { false };
    bool hasDepositCell { false };

    int carriedAmount { 0 };
    int capacity { 20 };
    int gatherTicks { 0 };
    int pathTicks { 0 };

    /*
     * Optimisation.
     * searchCooldownTicks évite que tous les collecteurs scannent la carte à chaque tick.
     * repathCooldownTicks évite de recalculer A* en boucle quand un chemin est bloqué.
     * pathFailCount permet d'abandonner une ressource après plusieurs échecs de chemin.
     */
    int searchCooldownTicks { 0 };
    int repathCooldownTicks { 0 };
    int pathFailCount { 0 };

    std::vector<Coordinate> aiPath;
    std::size_t aiPathIndex { 0 };

    bool isAdjacentTo(Coordinate a, Coordinate b) const;
    bool isTargetResourceValid(const MAP& map) const;

    bool selectNextResource(MAP& map);
    bool selectResourceOfType(MAP& map, ResourceType type);
    bool selectResourceOfTypeInRadius(MAP& map, ResourceType type, int radius);

    bool findDepositCell(MAP& map, std::vector<std::unique_ptr<Player>>& players);
    bool followCurrentPath(MAP& map, int tickRate);

    void clearPath();
    void releaseTargetReservation();
    void goSearch(int delayTicks = 0);
    void goDeposit();
    void depositCarriedResource(std::vector<std::unique_ptr<Player>>& players);

    void setSearchCooldown(int ticks);
    void setRepathCooldown(int tickRate);
    void updateCooldowns();
};
