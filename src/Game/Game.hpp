#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "../Frontend/Window/Window.hpp"
#include "../Frontend/Renderer/Renderer.hpp"
#include "../Frontend/EventManager/EventManager.hpp"
#include "../Frontend/SelectionManager/SelectionManager.hpp"
#include "../Frontend/UIManager/UIManager.hpp"
#include "../Frontend/Sound/Sound.hpp"

#include "../Backend/Map/Map.hpp"
#include "../Backend/Player/Player.hpp"
#include "../Backend/Unit/Unit.hpp"
#include "../Backend/Pathing/MassPath.hpp"

static constexpr int   TICK_RATE    { 10   };
static constexpr float TICK_DELAY   { 1000.0f / TICK_RATE };
static constexpr int   FPS_CAP      { 160 };
static constexpr float FRAME_DELAY  { 1000.0f / FPS_CAP };

class Building;

class Game
{
public:
    Game();
    virtual ~Game();

    void startGame();
    void stopGame();
    void run();
    void update();

private:
    /*
     * Initialisation et lancement de partie.
     */
    void initializeSDL();
    void initializeManagers();
    void shutdownSDL();
    void createMapFromInput();
    void createPlayersAndStartingBases(int menuChoice);
    void printPlayers() const;

    /*
     * Boucle principale.
     */
    void processEventsAndCommands();
    void processFixedTicks(float& tickAccumulator, int& tickCount);
    void renderFrameIfNeeded(Uint32& lastFrame, int& frameCount);
    void updateStatsIfNeeded(Uint32& lastStatsTime, int& frameCount, int& tickCount);

    /*
     * Commandes joueur.
     */
    void handleProductionRequests();
    void handleSoldierProductionRequest();
    void handleCollectorProductionRequest();
    void handleBuildingPlacement();
    void handleBuildingSelection();
    void handleOffensiveMoveOrder();
    void handleManualMoveOrder();

    /*
     * Simulation.
     */
    void updateEnemyStrategy();
    void updateManualPathing();
    void updateUnitAI();
    void updateUnitMovement();
    void updateCombat();
    void cleanupDestroyedEntities();
    void updateBuildingProductionAndSpawns();
    bool spawnPendingUnitFromBuilding(Building* building);

    /*
     * Rendu.
     */
    void renderFrame();
    void renderWorld();
    void renderUnits();
    void renderBuildingsLayer();
    void renderUI();

    /*
     * Outils.
     */
    Coordinate screenToCell(int screenX, int screenY) const;
    Coordinate resolveManualMoveTarget(Coordinate target) const;
    std::vector<Player*> getRawPlayers() const;

private:
    DISPLAY_OPTIONS options { 800, 600, false };

    std::unique_ptr<Window>           ptr_window;
    std::unique_ptr<Renderer>         ptr_renderer;
    std::unique_ptr<SelectionManager> ptr_selectionManager;
    std::unique_ptr<EventManager>     ptr_eventManager;
    std::unique_ptr<UIManager>        ptr_uiManager;
    std::unique_ptr<Map>              ptr_map;
    std::unique_ptr<Sound>            ptr_sound;

    std::vector<std::unique_ptr<Player>> ptr_players;
    std::vector<std::unique_ptr<Unit>>   ptr_units;

    int    MAP_W   { 0 };
    int    MAP_H   { 0 };
    bool   running { false };
    float  productionAccumulator { 0.0f };
    int    hudFPS  { 0 };
    int    hudTPS  { 0 };
    Uint64 currentTick { 0 };
};