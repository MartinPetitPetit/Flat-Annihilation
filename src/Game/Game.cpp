/*
 * Game.cpp
 *
 * Rôle général :
 * - initialise SDL, la fenêtre, le renderer, le son et les managers ;
 * - crée la carte, les joueurs et les bases de départ ;
 * - orchestre la boucle principale du jeu ;
 * - applique les commandes joueur, la simulation, le combat et le rendu.
 *
 * Important : ce fichier agit comme coordinateur central. Les détails
 * spécialisés restent dans les modules Backend et Frontend.
 */
#include "Game.hpp"
#include <iostream>
#include "../Backend/Building/Building.hpp"

#include "../Backend/AI/EnemyAI.hpp"
#include "../Backend/Building/Building.hpp"
#include "../Backend/Combat/CombatSystem.hpp"
#include "../Backend/Unit/Collector.hpp"

namespace
{
    /*
     * Vérifie si une unité peut apparaître sur une cellule précise.
     * La cellule doit être dans la carte, libre, praticable, sans bâtiment,
     * sans ressource et sans autre unité.
     */
    bool canSpawnUnitAt(const MAP& map, int x, int y)
    {
        if (!in_map(map, x, y)) {
            return false;
        }

        const Cell& cell = map[x][y];

        if (!cell.walkable) {
            return false;
        }

        if (cell.type_terrain != Plain) {
            return false;
        }

        if (cell.buildingID != -1) {
            return false;
        }

        if (cell.resource != nullptr) {
            return false;
        }

        if (cell.unit != nullptr) {
            return false;
        }

        return true;
    }

    /*
     * Cherche une cellule libre autour d'un bâtiment pour faire apparaître
     * une unité produite. La recherche se fait par anneaux successifs autour
     * de l'emprise du bâtiment.
     */
    bool findSpawnCellNearBuilding(
        const MAP& map,
        const Building* building,
        int& outX,
        int& outY
    ) {
        if (building == nullptr) {
            return false;
        }

        const BuildingDef& def = getBuildingDef(building->getType());

        int bx = building->getMapX();
        int by = building->getMapY();

        /*
         * Recherche une cellule libre autour du bâtiment.
         * On commence près du bâtiment, puis on agrandit le rayon.
         */
        for (int radius = 1; radius <= 6; radius++) {
            int minX = bx - radius;
            int maxX = bx + def.sizeX - 1 + radius;
            int minY = by - radius;
            int maxY = by + def.sizeY - 1 + radius;

            for (int x = minX; x <= maxX; x++) {
                for (int y = minY; y <= maxY; y++) {
                    bool onBorder =
                        x == minX ||
                        x == maxX ||
                        y == minY ||
                        y == maxY;

                    if (!onBorder) {
                        continue;
                    }

                    if (canSpawnUnitAt(map, x, y)) {
                        outX = x;
                        outY = y;
                        return true;
                    }
                }
            }
        }

        return false;
    }

    /*
     * Cherche une cellule libre autour d'une cellule déjà occupée.
     * Utilisé quand le joueur clique sur une unité ou une zone bloquée :
     * l'ordre est redirigé vers une position voisine praticable.
     */
    bool findFreeCellNearCell(
        const MAP& map,
        int centerX,
        int centerY,
        int& outX,
        int& outY
    ) {
        for (int radius = 1; radius <= 4; radius++) {
            for (int dx = -radius; dx <= radius; dx++) {
                for (int dy = -radius; dy <= radius; dy++) {
                    bool onBorder =
                        dx == -radius ||
                        dx == radius ||
                        dy == -radius ||
                        dy == radius;

                    if (!onBorder) {
                        continue;
                    }

                    int x = centerX + dx;
                    int y = centerY + dy;

                    if (canSpawnUnitAt(map, x, y)) {
                        outX = x;
                        outY = y;
                        return true;
                    }
                }
            }
        }

        return false;
    }

}

/*
 * Constructeur : prépare les bibliothèques SDL et crée les managers
 * nécessaires avant le lancement de la partie.
 */
Game::Game()
{
    initializeSDL();
    initializeManagers();
}

/*
 * Destructeur : libère les entités de jeu puis ferme proprement SDL,
 * SDL_ttf et SDL_image.
 */
Game::~Game()
{
    std::cout << "destruction de Game\n";

    ptr_players.clear();
    ptr_units.clear();
    ptr_map.reset();

    shutdownSDL();
}

/*
 * Initialise les sous-systèmes externes nécessaires au jeu : vidéo SDL,
 * rendu de texte TTF et chargement d'images PNG.
 */
void Game::initializeSDL()
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);
}

/*
 * Crée les objets principaux du Frontend : son, fenêtre, renderer, UI,
 * sélection et gestionnaire d'événements. C'est aussi ici que les sons
 * et la musique sont chargés.
 */
void Game::initializeManagers()
{
    ptr_sound = std::make_unique<Sound>();
    ptr_sound->load("click",  "assets/sounds/rhoo.wav");
    ptr_sound->load("hover",  "assets/sounds/ptiou.wav");
    ptr_sound->load("attack", "assets/sounds/attack.wav");
    ptr_sound->load("death",  "assets/sounds/death.wav");
    ptr_sound->load("spawn",  "assets/sounds/spawn.wav");
    ptr_sound->loadMusic("assets/sounds/Flat-construction-v2.wav");
    ptr_sound->playMusic();
    ptr_sound->setMusicVolume(200);

    ptr_window = std::make_unique<Window>("Flat Annihilation", options);
    ptr_renderer = std::make_unique<Renderer>(*ptr_window, "assets/fonts/Starjedi.ttf");
    ptr_uiManager = std::make_unique<UIManager>(*ptr_renderer, *ptr_window);
    ptr_selectionManager = std::make_unique<SelectionManager>();

    ptr_eventManager = std::make_unique<EventManager>(
        *ptr_selectionManager,
        *ptr_renderer,
        *ptr_uiManager,
        ptr_units
    );
}

/*
 * Ferme les bibliothèques SDL dans l'ordre inverse de leur initialisation.
 */
void Game::shutdownSDL()
{
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

/*
 * Lance le menu principal, puis initialise la carte et les joueurs si le
 * joueur choisit un mode de jeu valide.
 */
void Game::startGame()
{
    int choice = this->ptr_uiManager->showMainMenu(options, *ptr_sound);

    if (choice < 0) {
        return;
    }

    createMapFromInput();
    createPlayersAndStartingBases(choice);
    printPlayers();

    this->running = true;
    run();
}

/*
 * Demande les dimensions de la carte dans le terminal et crée l'objet Map.
 */
void Game::createMapFromInput()
{
    std::cout << "taille de la carte : MAP_W MAP_H = ";
    std::cin >> MAP_W >> MAP_H;

    this->ptr_map = std::make_unique<Map>(MAP_W, MAP_H);
}

/*
 * Crée le joueur humain et l'IA, puis demande au module Map de placer
 * leurs bases initiales de chaque côté de la carte.
 */
void Game::createPlayersAndStartingBases(int menuChoice)
{
    /*
     * Pour l'instant, une partie démarre toujours en 1v1 :
     * - joueur humain à gauche ;
     * - IA ennemie à droite.
     *
     * Le Game ne construit pas directement les bâtiments.
     * Il demande seulement au module Map de créer les bases initiales.
     */
    (void)menuChoice;

    ptr_players.clear();
    ptr_units.clear();

    ptr_players.push_back(std::make_unique<Player>());
    ptr_players.push_back(std::make_unique<Player>(0));

    bool playerBaseCreated = create_start_base(
        ptr_map->setGrid(),
        *ptr_players[0],
        ptr_units,
        StartBaseSide::Left,
        5
    );

    bool enemyBaseCreated = create_start_base(
        ptr_map->setGrid(),
        *ptr_players[1],
        ptr_units,
        StartBaseSide::Right,
        5
    );

    if (!playerBaseCreated) {
        std::cout << "Impossible de créer la base du joueur.\n";
    }

    if (!enemyBaseCreated) {
        std::cout << "Impossible de créer la base de l'IA.\n";
    }
}

/*
 * Affiche dans le terminal les joueurs actuellement présents dans la partie.
 */
void Game::printPlayers() const
{
    for (const auto& player : ptr_players) {
        if (player) {
            std::cout << "joueur : " << player->getName() << "\n";
        }
    }
}

/*
 * Arrête la partie courante et libère les données liées à la carte,
 * aux joueurs et aux unités.
 */
void Game::stopGame()
{
    ptr_players.clear();
    ptr_units.clear();
    ptr_map.reset();
    running = false;
}

/*
 * Boucle principale du jeu.
 * Elle sépare la simulation à fréquence fixe, le rendu limité par FPS_CAP
 * et la mise à jour des statistiques HUD.
 */
void Game::run()
{
    Uint32 lastTick = SDL_GetTicks();
    Uint32 lastFrame = SDL_GetTicks();
    Uint32 lastStatsTime = SDL_GetTicks();

    float tickAccumulator = 0.0f;

    int frameCount = 0;
    int tickCount = 0;

    while (running && !ptr_eventManager->isQuit()) {
        Uint32 now = SDL_GetTicks();
        float elapsed = static_cast<float>(now - lastTick);
        lastTick = now;

        tickAccumulator += elapsed;

        // Placement de bâtiment
        processEventsAndCommands();
        processFixedTicks(tickAccumulator, tickCount);
        renderFrameIfNeeded(lastFrame, frameCount);
        updateStatsIfNeeded(lastStatsTime, frameCount, tickCount);
    }

    std::cout << "\n";
}

/*
 * Lit les événements SDL puis transforme les demandes stockées par
 * EventManager/UIManager en actions concrètes de jeu.
 */
void Game::processEventsAndCommands()
{
    ptr_eventManager->pollEvents();

    handleProductionRequests();
    handleBuildingPlacement();
    handleBuildingSelection();
    handleOffensiveMoveOrder();
    handleManualMoveOrder();
}

/*
 * Regroupe les demandes de production venant du HUD.
 */
void Game::handleProductionRequests()
{
    handleSoldierProductionRequest();
    handleCollectorProductionRequest();
}

/*
 * Traite la production d'un soldat depuis une caserne sélectionnée.
 * Le coût est payé avant l'ajout à la file, puis remboursé si la file refuse.
 */
void Game::handleSoldierProductionRequest()
{
    if (!ptr_uiManager->pendingProduceUnit) {
        return;
    }

    ptr_uiManager->pendingProduceUnit = false;

    Building* selectedBuilding = ptr_uiManager->getSelectedBuilding();

    if (selectedBuilding == nullptr ||
        selectedBuilding->getType() != BuildingType::Barracks ||
        selectedBuilding->getTeam() != 0 ||
        ptr_players.empty()) {
        return;
    }

    Player* localPlayer = ptr_players[0].get();
    const int foodCost = 10;

    if (!localPlayer->spendFood(foodCost)) {
        return;
    }

    if (!selectedBuilding->queueUnit(UnitKind::Soldier)) {
        localPlayer->addFood(foodCost);
    }
}

/*
 * Traite la production d'un collecteur depuis le Town Center sélectionné.
 * Les ressources sont vérifiées puis dépensées avant la mise en file.
 */
void Game::handleCollectorProductionRequest()
{
    if (!ptr_uiManager->pendingProduceCollector) {
        return;
    }

    ptr_uiManager->pendingProduceCollector = false;

    Building* selectedBuilding = ptr_uiManager->getSelectedBuilding();

    if (selectedBuilding == nullptr ||
        selectedBuilding->getType() != BuildingType::TownCenter ||
        selectedBuilding->getTeam() != 0 ||
        ptr_players.empty()) {
        return;
    }

    Player* localPlayer = ptr_players[0].get();
    const int woodCost = 20;
    const int foodCost = 40;

    if (localPlayer->getWood() < woodCost ||
        localPlayer->getFood() < foodCost) {
        return;
    }

    localPlayer->spendWood(woodCost);
    localPlayer->spendFood(foodCost);

    if (!selectedBuilding->queueUnit(UnitKind::Collector)) {
        localPlayer->addWood(woodCost);
        localPlayer->addFood(foodCost);
    }
}

/*
 * Place un bâtiment demandé par le HUD en convertissant la position écran
 * du clic en cellule de carte.
 */
void Game::handleBuildingPlacement()
{
    if (!ptr_eventManager->pendingBuild || ptr_players.empty()) {
        return;
    }

    Coordinate cell = screenToCell(
        ptr_eventManager->pendingBuildX,
        ptr_eventManager->pendingBuildY
    );

    ptr_players[0]->placeBuilding(
        ptr_uiManager->getSelectedBuildingType(),
        cell.getX(),
        cell.getY(),
        ptr_map->setGrid()
    );

    ptr_uiManager->cancelBuildingMode();
    ptr_eventManager->consumeBuild();
}

/*
 * Sélectionne un bâtiment allié à partir d'un clic sur la carte.
 * Si un bâtiment est sélectionné, la sélection d'unités est vidée.
 */
void Game::handleBuildingSelection()
{
    if (!ptr_eventManager->pendingBuildingSelect || ptr_players.empty()) {
        return;
    }

    Coordinate cell = screenToCell(
        ptr_eventManager->pendingBuildingSelectX,
        ptr_eventManager->pendingBuildingSelectY
    );

    Building* selectedBuilding = nullptr;

    if (in_map(ptr_map->setGrid(), cell.getX(), cell.getY())) {
        int buildingID = ptr_map->setGrid()[cell.getX()][cell.getY()].buildingID;
        int ownerID = ptr_map->setGrid()[cell.getX()][cell.getY()].buildingOwner;

        if (buildingID != -1 &&
            ownerID == 0 &&
            ownerID < static_cast<int>(ptr_players.size())) {
            selectedBuilding = ptr_players[ownerID]->getBuilding(buildingID);
        }
    }

    ptr_uiManager->selectBuilding(selectedBuilding);

    if (selectedBuilding != nullptr) {
        ptr_selectionManager->clearSelection();
    }

    ptr_eventManager->consumeBuildingSelect();
}

/*
 * Applique un ordre offensif aux unités sélectionnées capables d'attaquer.
 * Les unités cherchent ensuite automatiquement les ennemis proches.
 */
void Game::handleOffensiveMoveOrder()
{
    if (!ptr_eventManager->pendingOffensiveMove) {
        return;
    }

    std::vector<Unit*>& selectedUnits = ptr_selectionManager->getSelected();

    if (!selectedUnits.empty()) {
        Coordinate offensiveTarget(
            ptr_eventManager->pendingOffensiveMoveX,
            ptr_eventManager->pendingOffensiveMoveY
        );

        for (Unit* unit : selectedUnits) {
            if (unit == nullptr) {
                continue;
            }

            if (unit->getTeam() != 0 || !unit->canAttack()) {
                continue;
            }

            unit->setOffensiveDestination(
                offensiveTarget,
                ptr_map->setGrid(),
                ptr_units
            );
        }
    }

    ptr_eventManager->consumeOffensiveMove();
}

/*
 * Applique un ordre de déplacement normal aux unités sélectionnées.
 * Le mode offensif est annulé avant de créer le plan de groupe.
 */
void Game::handleManualMoveOrder()
{
    if (!ptr_eventManager->pendingMove) {
        return;
    }

    std::vector<Unit*>& selectedUnits = ptr_selectionManager->getSelected();

    if (!selectedUnits.empty()) {
        Coordinate clickedTarget(
            ptr_eventManager->pendingMoveX,
            ptr_eventManager->pendingMoveY
        );

        Coordinate finalTarget = resolveManualMoveTarget(clickedTarget);

        for (Unit* unit : selectedUnits) {
            if (unit != nullptr) {
                unit->clearOffensiveMode();
            }
        }

        MassPath::requestGroupMove(
            ptr_map->setGrid(),
            selectedUnits,
            finalTarget
        );
    }

    ptr_eventManager->consumeMove();
}

/*
 * Exécute la simulation à fréquence fixe grâce à un accumulateur de temps.
 * Cela permet de garder une logique stable même si le rendu varie.
 */
void Game::processFixedTicks(float& tickAccumulator, int& tickCount)
{
    while (tickAccumulator >= TICK_DELAY) {
        if (!ptr_uiManager->isGamePaused()) {
            update();
            tickCount++;
        }

        tickAccumulator -= TICK_DELAY;

        if (tickAccumulator > TICK_DELAY * 5) {
            tickAccumulator = 0.0f;
            break;
        }
    }
}

/*
 * Rend une frame seulement si la limite de FPS le permet.
 */
void Game::renderFrameIfNeeded(Uint32& lastFrame, int& frameCount)
{
    Uint32 frameNow = SDL_GetTicks();

    if (FPS_CAP != 0 && static_cast<float>(frameNow - lastFrame) < FRAME_DELAY) {
        SDL_Delay(1);
        return;
    }

    renderFrame();

    lastFrame = frameNow;
    frameCount++;
}

/*
 * Met à jour les statistiques visibles dans le HUD et dans le terminal
 * une fois par seconde.
 */
void Game::updateStatsIfNeeded(Uint32& lastStatsTime, int& frameCount, int& tickCount)
{
    Uint32 statsNow = SDL_GetTicks();

    if (statsNow - lastStatsTime < 1000) {
        return;
    }

    hudFPS = frameCount;
    hudTPS = tickCount;

    ptr_uiManager->setHUDStats(
        hudFPS,
        hudTPS,
        currentTick,
        TICK_RATE
    );

    float gameTimeSeconds = static_cast<float>(currentTick) / TICK_RATE;

    int minutes = static_cast<int>(gameTimeSeconds) / 60;
    int seconds = static_cast<int>(gameTimeSeconds) % 60;

    std::cout << "FPS: " << frameCount
              << " | TPS: " << tickCount
              << " | tick: " << currentTick
              << " | temps: " << minutes << "m" << seconds << "s"
              << "\r" << std::flush;

    frameCount = 0;
    tickCount = 0;
    lastStatsTime = statsNow;
}

/*
 * Tick principal de simulation : stratégie IA, pathing, IA des unités,
 * mouvement, combat, nettoyage et production.
 */
void Game::update()
{
    currentTick++;
    // Production des bâtiments
    float dtSeconds = 1.0f / TICK_RATE;
    productionAccumulator += dtSeconds;

    updateEnemyStrategy();
    updateManualPathing();
    updateUnitAI();
    updateUnitMovement();
    updateCombat();
    cleanupDestroyedEntities();
    updateBuildingProductionAndSpawns();
}

/*
 * Met à jour la logique stratégique de toutes les IA ennemies.
 */
void Game::updateEnemyStrategy()
{
    for (int i = 1; i < static_cast<int>(ptr_players.size()); i++) {
        EnemyAI::updateSimpleEnemy(
            ptr_map->setGrid(),
            ptr_players,
            ptr_units,
            i,
            static_cast<int>(currentTick),
            TICK_RATE
        );
    }
}

/*
 * Donne au système de déplacement de groupe l'occasion de gérer les
 * recalculs et les échecs de déplacement.
 */
void Game::updateManualPathing()
{
    MassPath::processRepathRequests(ptr_map->setGrid());

    std::vector<Unit*> failedUnits;

    if (!MassPath::consumeFailedMove(failedUnits)) {
        return;
    }

    std::cout << "Rota impossível: unidades paradas.\n";

    for (Unit* unit : failedUnits) {
        if (!unit) {
            continue;
        }

        MassPath::clearPlan(unit);

        unit->setDestination(
            unit->getPos(),
            ptr_map->setGrid(),
            ptr_units
        );
    }
}

/*
 * Met à jour l'IA individuelle de chaque unité vivante.
 */
void Game::updateUnitAI()
{
    for (auto& unit : ptr_units) {
        if (!unit) {
            continue;
        }

        unit->updateAI(
            ptr_map->setGrid(),
            ptr_units,
            ptr_players,
            TICK_RATE
        );
    }
}

/*
 * Avance les unités vers leur destination selon leur logique de mouvement.
 */
void Game::updateUnitMovement()
{
    for (auto& unit : ptr_units) {
        if (!unit) {
            continue;
        }

        unit->updateMove(
            ptr_map->setGrid(),
            ptr_units,
            TICK_RATE
        );
    }
}



/*
 * Délègue la résolution des attaques au CombatSystem.
 */
void Game::updateCombat()
{
    CombatSystem::update(
        ptr_map->setGrid(),
        ptr_players,
        ptr_units,
        TICK_RATE,
        ptr_sound.get()
    );
}
/*
 * Nettoie les unités et bâtiments détruits, puis corrige les sélections
 * qui pointaient vers des entités mortes.
 */
void Game::cleanupDestroyedEntities()
{
    bool hasDeadUnit = false;

    for (const auto& unit : ptr_units) {
        if (unit && !unit->isAlive()) {
            hasDeadUnit = true;
            break;
        }
    }

    if (hasDeadUnit) {
        ptr_selectionManager->clearSelection();
    }

    Building* selectedBuilding = ptr_uiManager->getSelectedBuilding();

    if (selectedBuilding != nullptr && !selectedBuilding->isAlive()) {
        ptr_uiManager->clearBuildingSelection();
    }


    CombatSystem::removeDeadEntities(
        ptr_map->setGrid(),
        ptr_players,
        ptr_units,
        ptr_sound.get()
    );
}

/*
 * Avance les files de production des bâtiments et fait apparaître les
 * unités prêtes dès qu'une cellule libre est disponible.
 */
void Game::updateBuildingProductionAndSpawns()
{
    float dt = 1.0f / static_cast<float>(TICK_RATE);

    for (auto& player : ptr_players) {
        if (!player) {
            continue;
        }

        for (const auto& buildingPtr : player->getBuildings()) {
            Building* building = buildingPtr.get();

            if (building == nullptr || !building->isAlive()) {
                continue;
            }

            building->tick(dt);

            if (building->hasPendingSpawn()) {
                spawnPendingUnitFromBuilding(building);
            }
        }
    }
}

/*
 * Crée physiquement une unité produite par un bâtiment.
 * Les collecteurs utilisent la classe Collector, les soldats la classe Unit.
 */
bool Game::spawnPendingUnitFromBuilding(Building* building)
{
    int spawnX = 0;
    int spawnY = 0;

    if (!findSpawnCellNearBuilding(
            ptr_map->setGrid(),
            building,
            spawnX,
            spawnY
        )) {
        /*
         * Pas de place disponible.
         * On garde pendingSpawn à true pour réessayer au prochain tick.
         */
        return false;
    }

    int unitId = static_cast<int>(ptr_units.size());
    int team = building->getTeam();
    UnitKind kind = building->getPendingSpawnKind();

    if (kind == UnitKind::Collector) {
        ptr_units.push_back(
            std::make_unique<Collector>(unitId, team, spawnX, spawnY)
        );
    }
    else {
        ptr_units.push_back(
            std::make_unique<Unit>(unitId, team, spawnX, spawnY)
        );
    }

    ptr_map->setGrid()[spawnX][spawnY].unit = ptr_units.back().get();

    /*
     * Le son de création d'unité est réservé au joueur local.
     * Les productions de l'IA restent silencieuses pour éviter un retour audio
     * sur des actions qui ne sont pas directement déclenchées par le joueur.
     */
    if (ptr_sound && team == 0) {
        ptr_sound->play("spawn");
    }

    building->consumeSpawn();
    return true;
}

/*
 * Rend une frame complète : monde, unités, bâtiments, interface, puis present.
 */
void Game::renderFrame()
{
    ptr_renderer->clear();

    renderWorld();
    renderUnits();
    renderBuildingsLayer();
    renderUI();

    ptr_renderer->present();
}

/*
 * Rend la carte avec les options actuelles de fenêtre.
 */
void Game::renderWorld()
{
    DISPLAY_OPTIONS options = this->ptr_window->getOptions();
    ptr_renderer->drawMap(
        ptr_map->getGrid(),
        MAP_W,
        MAP_H,
        options
    );
}

/*
 * Rend toutes les unités présentes dans la partie.
 */
void Game::renderUnits()
{
    for (auto& unit : ptr_units) {
        if (!unit) {
            continue;
        }

        unit->render(
            ptr_renderer.get(),
            ptr_renderer->getOffsetX(),
            ptr_renderer->getOffsetY(),
            ptr_renderer->getScale()
        );
    }
}

/*
 * Rend les bâtiments au-dessus du terrain et avec les couleurs d'équipe.
 */
void Game::renderBuildingsLayer()
{
    ptr_uiManager->renderBuildings(
        ptr_map->getGrid(),
        getRawPlayers(),
        ptr_renderer->getScale(),
        ptr_renderer->getOffsetX(),
        ptr_renderer->getOffsetY()
    );
}

/*
 * Rend les éléments d'interface : rectangle de sélection, fantôme de
 * bâtiment et HUD principal.
 */
void Game::renderUI()
{
    ptr_uiManager->renderDragRect(*ptr_selectionManager);

    if (ptr_uiManager->isInBuildingMode()) {
        int mx = 0;
        int my = 0;
        SDL_GetMouseState(&mx, &my);

        ptr_uiManager->renderBuildingGhost(
            mx,
            my,
            ptr_uiManager->getSelectedBuildingType(),
            ptr_renderer->getScale(),
            ptr_renderer->getOffsetX(),
            ptr_renderer->getOffsetY()
        );
    }

    ptr_uiManager->renderHUD(
        ptr_players.empty() ? nullptr : ptr_players[0].get(),
        ptr_selectionManager->getSelected()
    );
}

/*
 * Convertit une position écran en coordonnée de cellule selon le zoom
 * et le décalage caméra actuels.
 */
Coordinate Game::screenToCell(int screenX, int screenY) const
{
    int scale = ptr_renderer->getScale();
    int offsetX = ptr_renderer->getOffsetX();
    int offsetY = ptr_renderer->getOffsetY();

    return Coordinate(
        (screenX - offsetX) / scale,
        (screenY - offsetY) / scale
    );
}



/*
 * Ajuste une destination de déplacement si le joueur a cliqué sur une
 * cellule occupée par un bâtiment ou une unité.
 */
Coordinate Game::resolveManualMoveTarget(Coordinate target) const
{
    if (!in_map(ptr_map->getGrid(), target.getX(), target.getY())) {
        return target;
    }

    const Cell& clickedCell = ptr_map->getGrid()[target.getX()][target.getY()];

    if (clickedCell.buildingID != -1 &&
        clickedCell.buildingOwner >= 0 &&
        clickedCell.buildingOwner < static_cast<int>(ptr_players.size())) {
        Building* building = ptr_players[clickedCell.buildingOwner]->getBuilding(clickedCell.buildingID);

        int x = 0;
        int y = 0;

        if (findSpawnCellNearBuilding(ptr_map->getGrid(), building, x, y)) {
            return Coordinate(x, y);
        }
    }

    if (clickedCell.unit != nullptr) {
        int x = 0;
        int y = 0;

        if (findFreeCellNearCell(
                ptr_map->getGrid(),
                target.getX(),
                target.getY(),
                x,
                y
            )) {
            return Coordinate(x, y);
        }
    }

    return target;
}

/*
 * Convertit les unique_ptr<Player> en pointeurs bruts pour les fonctions
 * de rendu qui ne prennent pas possession des objets.
 */
std::vector<Player*> Game::getRawPlayers() const
{
    std::vector<Player*> rawPlayers;

    for (const auto& player : ptr_players) {
        rawPlayers.push_back(player.get());
    }

    return rawPlayers;
}
