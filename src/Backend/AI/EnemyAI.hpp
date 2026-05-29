#pragma once

#include <memory>
#include <vector>

#include "../Map/Map.hpp"
#include "../Player/Player.hpp"
#include "../Unit/Unit.hpp"

class EnemyAI
{
public:
    struct Coefficients
    {
        int playerCollectors { 0 };
        int playerSoldiers   { 0 };
        int enemyCollectors  { 0 };
        int enemySoldiers    { 0 };

        /*
         * food_collecteur : objectif économique de l'IA.
         * Il suit le nombre de collecteurs du joueur.
         */
        int food_collecteur  { 0 };

        /*
         * force : score militaire simple.
         * Les soldats comptent plus que les collecteurs.
         */
        int playerForce      { 0 };
        int enemyForce       { 0 };
    };

    static void updateSimpleEnemy(
        MAP& map,
        std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<Unit>>& units,
        int enemyPlayerIndex,
        int currentTick,
        int tickRate
    );

    static Coefficients computeCoefficients(
        const std::vector<std::unique_ptr<Unit>>& units,
        int playerTeam,
        int enemyTeam
    );

private:
    static void updateEconomyAndProduction(
        MAP& map,
        std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<Unit>>& units,
        int enemyPlayerIndex
    );

    static void updateTroopControl(
        MAP& map,
        std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<Unit>>& units,
        int enemyPlayerIndex
    );
};
