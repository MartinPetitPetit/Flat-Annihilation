/*
 * Interface du module CombatSystem.
 * Elle expose les fonctions statiques responsables de la résolution des combats.
 */


#include <memory>
#include <vector>

#include "../Map/Map.hpp"
#include "../Player/Player.hpp"
#include "../Unit/Unit.hpp"
#include "../../Frontend/Sound/Sound.hpp"

/* Système statique responsable de la résolution des combats. */
class CombatSystem
{
public:
    /* Applique les attaques périodiques des unités contre les ennemis à portée. */
    static void update(
        MAP& map,
        std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<Unit>>& units,
        int tickRate,
        Sound* sound = nullptr
    );

    /* Nettoie les entités mortes et indique si la partie doit être terminée. */
    static bool removeDeadEntities(
        MAP& map,
        std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<Unit>>& units,
        Sound* sound = nullptr
    );
};