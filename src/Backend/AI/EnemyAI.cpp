/*
 * EnemyAI.cpp
 * Implémente la logique de l'intelligence artificielle ennemie :
 * économie, production, construction de caserne, défense et attaque.
 */

#include "EnemyAI.hpp"

/* Dépendances nécessaires pour manipuler bâtiments et collecteurs. */
#include "../Building/Building.hpp"
#include "../Unit/Collector.hpp"

/* Bibliothèques standards utilisées pour max() et les valeurs limites. */
#include <algorithm>
#include <limits>

/*
 * Les fonctions dans ce namespace anonyme sont privées à ce fichier.
 * Elles servent de petits outils pour garder EnemyAI plus lisible.
 */
namespace
{
    /* Paramètres principaux de coût, rythme de décision et comportement stratégique. */
    constexpr int START_COLLECTORS = 5;
    constexpr int COLLECTOR_WOOD_COST = 20;
    constexpr int COLLECTOR_FOOD_COST = 40;
    constexpr int SOLDIER_FOOD_COST = 10;

    /*
     * L'IA stratégique ne doit pas réfléchir à chaque tick.
     * Toutes les décisions lourdes passent par ce délai.
     */
    constexpr int AI_UPDATE_SECONDS = 2;

    /*
     * Zone de défense autour du Town Center de l'IA.
     * Si une unité du joueur entre dans cette zone, les soldats défendent.
     */
    constexpr int BASE_DEFENSE_RADIUS = 18;

    /*
     * Zone offensive autour du Town Center du joueur.
     * Quand la base IA n'est pas menacée, les soldats IA visent d'abord
     * les unités présentes dans cette zone.
     */
    constexpr int ENEMY_BASE_TARGET_RADIUS = 20;

    /*
     * L'IA attend d'avoir au moins quelques soldats avant d'être vraiment
     * dangereuse, mais elle peut envoyer moins de soldats si elle n'a que cela.
     */
    constexpr int MIN_ATTACK_SOLDIERS = 3;

    /* Retourne la valeur absolue d'un entier sans dépendre d'une autre bibliothèque. */
    int absInt(int value)
    {
        return value < 0 ? -value : value;
    }

    /*
     * Distance de Chebyshev : utile sur une grille où le déplacement diagonal
     * compte comme un seul pas.
     */
    int chebyshevDistance(Coordinate a, Coordinate b)
    {
        int dx = absInt(a.getX() - b.getX());
        int dy = absInt(a.getY() - b.getY());
        return std::max(dx, dy);
    }

    /* Calcule une position centrale approximative pour viser un bâtiment. */
    Coordinate buildingCenter(const Building* building)
    {
        if (building == nullptr) {
            return Coordinate(0, 0);
        }

        const BuildingDef& def = getBuildingDef(building->getType());

        return Coordinate(
            building->getMapX() + def.sizeX / 2,
            building->getMapY() + def.sizeY / 2
        );
    }

    /*
     * Calcule la distance entre une case et la surface d'un bâtiment.
     * Si la case est déjà dans le rectangle du bâtiment, la distance vaut 0.
     */
    int distanceToBuilding(Coordinate pos, const Building* building)
    {
        if (building == nullptr) {
            return std::numeric_limits<int>::max();
        }

        const BuildingDef& def = getBuildingDef(building->getType());

        int minX = building->getMapX();
        int maxX = building->getMapX() + def.sizeX - 1;
        int minY = building->getMapY();
        int maxY = building->getMapY() + def.sizeY - 1;

        int dx = 0;
        int dy = 0;

        if (pos.getX() < minX) {
            dx = minX - pos.getX();
        }
        else if (pos.getX() > maxX) {
            dx = pos.getX() - maxX;
        }

        if (pos.getY() < minY) {
            dy = minY - pos.getY();
        }
        else if (pos.getY() > maxY) {
            dy = pos.getY() - maxY;
        }

        return std::max(dx, dy);
    }

    /* Vérifie si toutes les cases nécessaires sont libres pour placer un bâtiment. */
    bool canPlaceBuildingAt(const MAP& map, BuildingType type, int x, int y)
    {
        const BuildingDef& def = getBuildingDef(type);

        for (int dx = 0; dx < def.sizeX; dx++) {
            for (int dy = 0; dy < def.sizeY; dy++) {
                int px = x + dx;
                int py = y + dy;

                if (!in_map(map, px, py)) {
                    return false;
                }

                const Cell& cell = map[px][py];

                if (cell.type_terrain != Plain ||
                    cell.buildingID != -1 ||
                    cell.resource != nullptr ||
                    cell.unit != nullptr) {
                    return false;
                }
            }
        }

        return true;
    }

    /*
     * Cherche progressivement autour d'un point une position valide pour construire.
     * Le parcours par rayons évite de scanner toute la carte inutilement.
     */
    bool findBuildingSpotNear(
        const MAP& map,
        BuildingType type,
        int centerX,
        int centerY,
        int& outX,
        int& outY
    ) {
        for (int radius = 0; radius <= 24; radius++) {
            for (int dx = -radius; dx <= radius; dx++) {
                for (int dy = -radius; dy <= radius; dy++) {
                    bool border =
                        dx == -radius ||
                        dx == radius ||
                        dy == -radius ||
                        dy == radius;

                    if (!border && radius != 0) {
                        continue;
                    }

                    int x = centerX + dx;
                    int y = centerY + dy;

                    if (canPlaceBuildingAt(map, type, x, y)) {
                        outX = x;
                        outY = y;
                        return true;
                    }
                }
            }
        }

        return false;
    }

    /* Retourne le premier bâtiment vivant du type demandé pour un joueur donné. */
    Building* findFirstBuilding(Player& player, BuildingType type)
    {
        for (const auto& building : player.getBuildings()) {
            if (building && building->isAlive() && building->getType() == type) {
                return building.get();
            }
        }

        return nullptr;
    }

    /* Choisit le bâtiment prioritaire à attaquer : Town Center puis autre bâtiment vivant. */
    Building* findPrimaryEnemyTarget(Player& player)
    {
        Building* townCenter = findFirstBuilding(player, BuildingType::TownCenter);

        if (townCenter != nullptr) {
            return townCenter;
        }

        for (const auto& building : player.getBuildings()) {
            if (building && building->isAlive()) {
                return building.get();
            }
        }

        return nullptr;
    }

    /*
     * Récupère les unités offensives de l'IA.
     * Les collecteurs sont volontairement exclus des groupes d'attaque.
     */
    std::vector<Unit*> collectSoldiers(
        std::vector<std::unique_ptr<Unit>>& units,
        int team
    ) {
        std::vector<Unit*> soldiers;

        for (auto& unitPtr : units) {
            Unit* unit = unitPtr.get();

            if (unit == nullptr || !unit->isAlive()) {
                continue;
            }

            if (unit->getTeam() != team) {
                continue;
            }

            if (!unit->canAttack()) {
                continue;
            }

            /*
             * Les collecteurs ne doivent jamais être utilisés comme troupe
             * d'attaque, même s'ils appartiennent à l'IA.
             */
            if (dynamic_cast<Collector*>(unit) != nullptr) {
                continue;
            }

            soldiers.push_back(unit);
        }

        return soldiers;
    }

    /* Compte les unités adverses dans un rayon autour d'un bâtiment de référence. */
    int countEnemyUnitsNearBase(
        const std::vector<std::unique_ptr<Unit>>& units,
        int aiTeam,
        const Building* base,
        int radius
    ) {
        if (base == nullptr) {
            return 0;
        }

        int count = 0;

        for (const auto& unitPtr : units) {
            const Unit* unit = unitPtr.get();

            if (unit == nullptr || !unit->isAlive()) {
                continue;
            }

            if (unit->getTeam() == aiTeam) {
                continue;
            }

            if (distanceToBuilding(unit->getPos(), base) <= radius) {
                count++;
            }
        }

        return count;
    }

    /* Trouve l'unité ennemie la plus proche d'une base dans un rayon donné. */
    Unit* findClosestEnemyUnitNearBase(
        const std::vector<std::unique_ptr<Unit>>& units,
        int aiTeam,
        const Building* base,
        int radius
    ) {
        Unit* best = nullptr;
        int bestDistanceToBase = std::numeric_limits<int>::max();

        if (base == nullptr) {
            return nullptr;
        }

        for (const auto& unitPtr : units) {
            Unit* unit = unitPtr.get();

            if (unit == nullptr || !unit->isAlive()) {
                continue;
            }

            if (unit->getTeam() == aiTeam) {
                continue;
            }

            int distance = distanceToBuilding(unit->getPos(), base);

            if (distance > radius) {
                continue;
            }

            if (best == nullptr || distance < bestDistanceToBase) {
                best = unit;
                bestDistanceToBase = distance;
            }
        }

        return best;
    }

    /* Trouve l'unité ennemie la plus proche d'un point de la carte. */
    Unit* findClosestEnemyUnitToPoint(
        const std::vector<std::unique_ptr<Unit>>& units,
        int aiTeam,
        Coordinate point
    ) {
        Unit* best = nullptr;
        int bestDistance = std::numeric_limits<int>::max();

        for (const auto& unitPtr : units) {
            Unit* unit = unitPtr.get();

            if (unit == nullptr || !unit->isAlive()) {
                continue;
            }

            if (unit->getTeam() == aiTeam) {
                continue;
            }

            int distance = chebyshevDistance(unit->getPos(), point);

            if (best == nullptr || distance < bestDistance) {
                best = unit;
                bestDistance = distance;
            }
        }

        return best;
    }

    /* Envoie chaque soldat valide vers une destination offensive commune. */
    void sendSoldiersToTarget(
        const std::vector<Unit*>& soldiers,
        Coordinate target,
        MAP& map,
        const std::vector<std::unique_ptr<Unit>>& units
    ) {
        for (Unit* soldier : soldiers) {
            if (soldier == nullptr || !soldier->isAlive() || !soldier->canAttack()) {
                continue;
            }

            soldier->setOffensiveDestination(
                target,
                map,
                units
            );
        }
    }

    /*
     * Ajoute un collecteur à la file du Town Center si les ressources et la file le permettent.
     * En cas d'échec après paiement, les ressources sont remboursées.
     */
    bool queueCollector(Player& player, Building* townCenter)
    {
        if (townCenter == nullptr) {
            return false;
        }

        if (townCenter->getQueueSize() >= townCenter->getMaxQueue()) {
            return false;
        }

        if (player.getWood() < COLLECTOR_WOOD_COST ||
            player.getFood() < COLLECTOR_FOOD_COST) {
            return false;
        }

        player.spendWood(COLLECTOR_WOOD_COST);
        player.spendFood(COLLECTOR_FOOD_COST);

        if (!townCenter->queueUnit(UnitKind::Collector)) {
            player.addWood(COLLECTOR_WOOD_COST);
            player.addFood(COLLECTOR_FOOD_COST);
            return false;
        }

        return true;
    }

    /*
     * Ajoute un soldat à la file de la caserne si l'IA possède assez de nourriture.
     * La nourriture est remboursée si la mise en file échoue.
     */
    bool queueSoldier(Player& player, Building* barracks)
    {
        if (barracks == nullptr) {
            return false;
        }

        if (barracks->getQueueSize() >= barracks->getMaxQueue()) {
            return false;
        }

        if (!player.spendFood(SOLDIER_FOOD_COST)) {
            return false;
        }

        if (!barracks->queueUnit(UnitKind::Soldier)) {
            player.addFood(SOLDIER_FOOD_COST);
            return false;
        }

        return true;
    }

    /* Cherche un emplacement proche du Town Center et tente d'y construire une caserne. */
    bool tryBuildBarracksNearTownCenter(MAP& map, Player& player, Building* townCenter)
    {
        if (townCenter == nullptr) {
            return false;
        }

        int buildX = 0;
        int buildY = 0;

        int direction = player.getId() == 0 ? 5 : -5;
        int centerX = townCenter->getMapX() + direction;
        int centerY = townCenter->getMapY();

        if (!findBuildingSpotNear(map, BuildingType::Barracks, centerX, centerY, buildX, buildY)) {
            return false;
        }

        return player.placeBuilding(BuildingType::Barracks, buildX, buildY, map);
    }

    /*
     * Détermine combien de soldats l'IA souhaite avoir avant ou pendant une offensive.
     * Le calcul dépend de la présence ennemie près de la base du joueur et de l'armée du joueur.
     */
    int desiredSoldierCountForAttack(
        const std::vector<std::unique_ptr<Unit>>& units,
        int aiTeam,
        const Building* playerTownCenter,
        const EnemyAI::Coefficients& coef
    ) {
        int unitsNearPlayerTown = countEnemyUnitsNearBase(
            units,
            aiTeam,
            playerTownCenter,
            ENEMY_BASE_TARGET_RADIUS
        );

        /*
         * L'IA veut assez de soldats pour attaquer les unités présentes
         * autour du Town Center du joueur, avec une petite marge.
         */
        int targetFromEnemyBase = unitsNearPlayerTown + 2;

        /*
         * Elle garde aussi une logique d'égalisation simple avec la force
         * militaire du joueur.
         */
        int targetFromPlayerArmy = coef.playerSoldiers + 1;

        return std::max(
            MIN_ATTACK_SOLDIERS,
            std::max(targetFromEnemyBase, targetFromPlayerArmy)
        );
    }
}

/*
 * Analyse toutes les unités vivantes et produit les compteurs utilisés par les décisions de l'IA.
 */
EnemyAI::Coefficients EnemyAI::computeCoefficients(
    const std::vector<std::unique_ptr<Unit>>& units,
    int playerTeam,
    int enemyTeam
) {
    Coefficients c;

    for (const auto& unit : units) {
        if (!unit || !unit->isAlive()) {
            continue;
        }

        bool isCollector = dynamic_cast<const Collector*>(unit.get()) != nullptr;

        if (unit->getTeam() == playerTeam) {
            if (isCollector) {
                c.playerCollectors++;
            }
            else {
                c.playerSoldiers++;
            }
        }
        else if (unit->getTeam() == enemyTeam) {
            if (isCollector) {
                c.enemyCollectors++;
            }
            else {
                c.enemySoldiers++;
            }
        }
    }

    c.food_collecteur = std::max(START_COLLECTORS, c.playerCollectors);

    /*
     * Score simple : un soldat compte 3 points, un collecteur 1 point.
     */
    c.playerForce = c.playerSoldiers * 3 + c.playerCollectors;
    c.enemyForce  = c.enemySoldiers  * 3 + c.enemyCollectors;

    return c;
}

/*
 * Fonction principale appelée par la boucle de jeu pour mettre à jour une IA ennemie.
 * Elle vérifie l'indice du joueur IA et évite de recalculer la stratégie à chaque tick.
 */
void EnemyAI::updateSimpleEnemy(
    MAP& map,
    std::vector<std::unique_ptr<Player>>& players,
    std::vector<std::unique_ptr<Unit>>& units,
    int enemyPlayerIndex,
    int currentTick,
    int tickRate
) {
    if (enemyPlayerIndex <= 0 ||
        enemyPlayerIndex >= static_cast<int>(players.size()) ||
        !players[0] ||
        !players[enemyPlayerIndex]) {
        return;
    }

    int updateDelay = std::max(1, tickRate * AI_UPDATE_SECONDS);

    /*
     * Décalage par IA : plusieurs IA ne réfléchissent pas toutes au même tick.
     */
    if ((currentTick + enemyPlayerIndex * 7) % updateDelay != 0) {
        return;
    }

    updateEconomyAndProduction(
        map,
        players,
        units,
        enemyPlayerIndex
    );

    updateTroopControl(
        map,
        players,
        units,
        enemyPlayerIndex
    );
}

/*
 * Décide quoi produire ou construire : collecteurs, caserne et soldats.
 * La priorité change si la base ennemie est menacée par le joueur.
 */
void EnemyAI::updateEconomyAndProduction(
    MAP& map,
    std::vector<std::unique_ptr<Player>>& players,
    std::vector<std::unique_ptr<Unit>>& units,
    int enemyPlayerIndex
) {
    Player& player = *players[0];
    Player& enemy = *players[enemyPlayerIndex];

    Building* enemyTownCenter = findFirstBuilding(enemy, BuildingType::TownCenter);

    if (enemyTownCenter == nullptr) {
        return;
    }

    Coefficients coef = computeCoefficients(
        units,
        player.getId(),
        enemy.getId()
    );

    Building* enemyBarracks = findFirstBuilding(enemy, BuildingType::Barracks);
    Building* playerTownCenter = findFirstBuilding(player, BuildingType::TownCenter);

    bool baseUnderThreat =
        findClosestEnemyUnitNearBase(
            units,
            enemy.getId(),
            enemyTownCenter,
            BASE_DEFENSE_RADIUS
        ) != nullptr;

    int wantedSoldiers = desiredSoldierCountForAttack(
        units,
        enemy.getId(),
        playerTownCenter,
        coef
    );

    /*
     * Priorité absolue : garder au moins quelques collecteurs.
     * Sans économie, l'IA ne pourra plus produire de soldats.
     */
    if (coef.enemyCollectors < START_COLLECTORS) {
        if (queueCollector(enemy, enemyTownCenter)) {
            return;
        }
    }

    /*
     * Si la base est menacée, l'IA produit en priorité des soldats.
     */
    if (baseUnderThreat) {
        if (enemyBarracks == nullptr) {
            if (tryBuildBarracksNearTownCenter(map, enemy, enemyTownCenter)) {
                return;
            }
        }
        else if (queueSoldier(enemy, enemyBarracks)) {
            return;
        }

        /*
         * Si elle ne peut pas produire de soldat, elle tente d'améliorer
         * l'économie pour pouvoir réagir plus tard.
         */
        if (coef.enemyCollectors < coef.food_collecteur) {
            queueCollector(enemy, enemyTownCenter);
        }

        return;
    }

    /*
     * Si la base n'est pas menacée, l'IA prépare une attaque vers la base
     * du joueur. Elle construit une caserne puis produit des soldats jusqu'à
     * atteindre l'objectif calculé.
     */
    if (enemyBarracks == nullptr) {
        if (tryBuildBarracksNearTownCenter(map, enemy, enemyTownCenter)) {
            return;
        }
    }

    if (enemyBarracks != nullptr && coef.enemySoldiers < wantedSoldiers) {
        if (queueSoldier(enemy, enemyBarracks)) {
            return;
        }
    }

    /*
     * Égalisation économique : après la priorité militaire, l'IA tente
     * de suivre le nombre de collecteurs du joueur.
     */
    if (coef.enemyCollectors < coef.food_collecteur) {
        if (queueCollector(enemy, enemyTownCenter)) {
            return;
        }
    }

    /*
     * Si tout est stable, elle garde une petite avance économique.
     */
    if (coef.enemyCollectors < coef.food_collecteur + 2) {
        queueCollector(enemy, enemyTownCenter);
    }
}

/*
 * Donne les ordres militaires aux soldats de l'IA.
 * La défense de la base passe avant l'attaque de la base du joueur.
 */
void EnemyAI::updateTroopControl(
    MAP& map,
    std::vector<std::unique_ptr<Player>>& players,
    std::vector<std::unique_ptr<Unit>>& units,
    int enemyPlayerIndex
) {
    if (enemyPlayerIndex <= 0 ||
        enemyPlayerIndex >= static_cast<int>(players.size()) ||
        !players[0] ||
        !players[enemyPlayerIndex]) {
        return;
    }

    Player& player = *players[0];
    Player& enemy = *players[enemyPlayerIndex];

    Building* enemyTownCenter = findFirstBuilding(enemy, BuildingType::TownCenter);

    if (enemyTownCenter == nullptr) {
        return;
    }

    std::vector<Unit*> soldiers = collectSoldiers(units, enemy.getId());

    if (soldiers.empty()) {
        return;
    }

    /*
     * Défense prioritaire : si le joueur approche la base de l'IA,
     * les soldats IA attaquent cette menace avant toute offensive.
     */
    Unit* defensiveTarget = findClosestEnemyUnitNearBase(
        units,
        enemy.getId(),
        enemyTownCenter,
        BASE_DEFENSE_RADIUS
    );

    if (defensiveTarget != nullptr) {
        sendSoldiersToTarget(
            soldiers,
            defensiveTarget->getPos(),
            map,
            units
        );
        return;
    }

    /*
     * Pas de menace proche : attaque de la base du joueur.
     * L'objectif prioritaire est une unité du joueur proche de son Town Center.
     * S'il n'y a aucune unité près du Town Center, les soldats visent le
     * Town Center lui-même, puis un autre bâtiment, puis une unité isolée.
     */
    Building* playerTownCenter = findFirstBuilding(player, BuildingType::TownCenter);

    Unit* targetNearPlayerTown = findClosestEnemyUnitNearBase(
        units,
        enemy.getId(),
        playerTownCenter,
        ENEMY_BASE_TARGET_RADIUS
    );

    if (targetNearPlayerTown != nullptr) {
        sendSoldiersToTarget(
            soldiers,
            targetNearPlayerTown->getPos(),
            map,
            units
        );
        return;
    }

    Building* offensiveBuilding = findPrimaryEnemyTarget(player);

    if (offensiveBuilding != nullptr) {
        sendSoldiersToTarget(
            soldiers,
            buildingCenter(offensiveBuilding),
            map,
            units
        );
        return;
    }

    Unit* fallbackUnit = findClosestEnemyUnitToPoint(
        units,
        enemy.getId(),
        buildingCenter(enemyTownCenter)
    );

    if (fallbackUnit != nullptr) {
        sendSoldiersToTarget(
            soldiers,
            fallbackUnit->getPos(),
            map,
            units
        );
    }
}
