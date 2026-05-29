#pragma once

/*
 * EnemyAI.hpp
 * Déclare la classe responsable des décisions de l'intelligence artificielle ennemie.
 * Ce fichier expose seulement l'interface publique et cache les étapes internes.
 */

#include <memory>
#include <vector>

/* Dépendances principales : carte, joueurs et unités manipulés par l'IA. */
#include "../Map/Map.hpp"
#include "../Player/Player.hpp"
#include "../Unit/Unit.hpp"

/*
 * EnemyAI regroupe les fonctions statiques utilisées par le jeu pour piloter
 * un joueur contrôlé par l'ordinateur.
 */
class EnemyAI
{
public:
    /*
     * Regroupe les compteurs utilisés pour comparer l'économie et l'armée
     * du joueur humain avec celles de l'IA.
     */
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

    /*
     * Point d'entrée principal appelé régulièrement par la boucle de jeu.
     * Il limite la fréquence des décisions puis lance économie et contrôle militaire.
     */
    static void updateSimpleEnemy(
        MAP& map,
        std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<Unit>>& units,
        int enemyPlayerIndex,
        int currentTick,
        int tickRate
    );

    /*
     * Calcule les indicateurs simples de force et d'économie à partir des unités vivantes.
     */
    static Coefficients computeCoefficients(
        const std::vector<std::unique_ptr<Unit>>& units,
        int playerTeam,
        int enemyTeam
    );

private:
    /* Les fonctions suivantes sont internes : elles séparent la production et les ordres militaires. */
private:
    /*
     * Gère les décisions économiques : collecteurs, caserne et production de soldats.
     */
    static void updateEconomyAndProduction(
        MAP& map,
        std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<Unit>>& units,
        int enemyPlayerIndex
    );

    /*
     * Gère les ordres donnés aux soldats : défendre la base ou attaquer le joueur.
     */
    static void updateTroopControl(
        MAP& map,
        std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<Unit>>& units,
        int enemyPlayerIndex
    );
};
