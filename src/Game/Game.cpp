#include "Game.hpp"

#include "../Backend/Pathing/MassPath.hpp"

Game::Game()
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    ptr_sound = std::make_unique<Sound>();
    ptr_sound->load("click",  "sounds/rhoo.wav");
    ptr_sound->load("hover",  "sounds/ptiou.wav");
    ptr_sound->loadMusic("sounds/Flat-construction-v2.wav");
    ptr_sound->playMusic();
    ptr_sound->setMusicVolume(200);

    ptr_window = std::make_unique<Window>("Flat Annihilation", options);
    ptr_renderer = std::make_unique<Renderer>(*ptr_window, "Starjedi.ttf");
    ptr_uiManager = std::make_unique<UIManager>(*ptr_renderer, *ptr_window);
    ptr_selectionManager = std::make_unique<SelectionManager>();

    ptr_eventManager = std::make_unique<EventManager>(
        *ptr_selectionManager,
        *ptr_renderer,
        *ptr_uiManager,
        ptr_units
    );
}

Game::~Game()
{
    std::cout << "destruction de Game\n";

    ptr_players.clear();
    ptr_map.reset();

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void Game::startGame()
{
    int choice = this->ptr_uiManager->showMainMenu(options, *ptr_sound);

    if (choice < 0) {
        return;
    }

    std::cout << "taille de la carte : MAP_W MAP_H = ";
    std::cin >> MAP_W >> MAP_H;

    this->ptr_map = std::make_unique<Map>(MAP_W, MAP_H);

    /*
     * Unités de test.
     */
    for (int i = 0; i < 15; i++) {
        int x = MAP_W / 2 + 5;
        int y = MAP_H / 2 + i;

        int unitId = static_cast<int>(ptr_units.size());

        ptr_units.push_back(
            std::make_unique<Unit>(unitId, 1, x, y)
        );

        if (in_map(ptr_map->data(), x, y)) {
            ptr_map->data()[x][y].unit = ptr_units.back().get();
        }
    }

    this->ptr_players.push_back(std::make_unique<Player>());

    int nbIA = 0;

    std::cout << "combien de joueurs IA ? ";
    std::cin >> nbIA;

    for (int i = 0; i < nbIA; i++) {
        this->ptr_players.push_back(std::make_unique<Player>(i));
    }

    for (auto& p : this->ptr_players) {
        std::cout << "joueur : " << p->getName() << "\n";
    }

    this->running = true;
    run();
}

void Game::stopGame()
{
    this->ptr_players.clear();
    this->ptr_map.reset();
    this->running = false;
}

void Game::run()
{
    Uint32 lastTick = SDL_GetTicks();
    Uint32 lastFrame = SDL_GetTicks();
    Uint32 lastStatsTime = SDL_GetTicks();

    float tickAccumulator = 0.0f;

    int frameCount = 0;
    int tickCount = 0;

    while (running && !ptr_eventManager->isQuit())
    {
        Uint32 now = SDL_GetTicks();
        float elapsed = static_cast<float>(now - lastTick);
        lastTick = now;

        tickAccumulator += elapsed;

        this->ptr_eventManager->pollEvents();

        /*
         * Placement de bâtiment.
         */
        if (ptr_eventManager->pendingBuild && !ptr_players.empty()) {
            int mx = ptr_eventManager->pendingBuildX;
            int my = ptr_eventManager->pendingBuildY;

            int scale = ptr_renderer->getScale();
            int offsetX = ptr_renderer->getOffsetX();
            int offsetY = ptr_renderer->getOffsetY();

            int cellX = (mx - offsetX) / scale;
            int cellY = (my - offsetY) / scale;

            ptr_players[0]->placeBuilding(
                ptr_uiManager->getSelectedBuildingType(),
                                          cellX,
                                          cellY,
                                          ptr_map->setGrid()
            );

            ptr_uiManager->cancelBuildingMode();
            ptr_eventManager->consumeBuild();
        }

        /*
         * Ordre de déplacement.
         *
         * Nova lógica:
         * - clique NÃO calcula A* inteiro;
         * - clique só agenda uma busca incremental;
         * - novo clique cancela a busca anterior;
         * - só o último clique continua.
         *
         * As unidades continuam com o plano antigo até a nova rota ficar pronta.
         */
        if (ptr_eventManager->pendingMove) {
            int mx = ptr_eventManager->pendingMoveX;
            int my = ptr_eventManager->pendingMoveY;

            std::vector<Unit*>& sel = ptr_selectionManager->getSelected();

            if (!sel.empty()) {
                MassPath::requestGroupMove(
                    ptr_map->setGrid(),
                                           sel,
                                           Coordinate(mx, my)
                );
            }

            ptr_eventManager->consumeMove();
        }

        /*
         * Ticks de jeu.
         */
        while (tickAccumulator >= TICK_DELAY)
        {
            if (!ptr_uiManager->isGamePaused()) {
                this->update();
            }

            tickAccumulator -= TICK_DELAY;

            if (!ptr_uiManager->isGamePaused()) {
                tickCount++;
            }

            if (tickAccumulator > TICK_DELAY * 5) {
                tickAccumulator = 0;
                break;
            }
        }

        /*
         * Rendu.
         */
        Uint32 frameNow = SDL_GetTicks();

        if (FPS_CAP == 0 || static_cast<float>(frameNow - lastFrame) >= FRAME_DELAY)
        {
            this->ptr_renderer->clear();

            std::vector<Player*> rawPlayers;

            for (auto& p : ptr_players) {
                rawPlayers.push_back(p.get());
            }

            this->ptr_renderer->drawMap(
                ptr_map->getGrid(),
                                        MAP_W,
                                        MAP_H,
                                        options
            );

            for (auto& u : ptr_units) {
                u->render(
                    ptr_renderer.get(),
                          ptr_renderer->getOffsetX(),
                          ptr_renderer->getOffsetY(),
                          ptr_renderer->getScale()
                );
            }

            this->ptr_uiManager->renderBuildings(
                ptr_map->getGrid(),
                                                 rawPlayers,
                                                 ptr_renderer->getScale(),
                                                 ptr_renderer->getOffsetX(),
                                                 ptr_renderer->getOffsetY()
            );

            this->ptr_uiManager->renderDragRect(*ptr_selectionManager);

            if (ptr_uiManager->isInBuildingMode()) {
                int mx, my;
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

            this->ptr_uiManager->renderHUD(
                ptr_players.empty() ? nullptr : ptr_players[0].get(),
                                           ptr_selectionManager->getSelected()
            );

            this->ptr_renderer->present();

            lastFrame = frameNow;
            frameCount++;
        }
        else
        {
            SDL_Delay(1);
        }

        /*
         * Stats FPS / TPS.
         */
        Uint32 statsNow = SDL_GetTicks();

        if (statsNow - lastStatsTime >= 1000)
        {
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
    }

    std::cout << "\n";
}

void Game::update()
{
    currentTick++;

    /*
     * Primeiro processa um pedaço do A*.
     * Assim, se uma rota ficar pronta, a unidade já pode usar no mesmo tick.
     */
    MassPath::processRepathRequests(ptr_map->setGrid());

    /*
     * Se uma rota falhou, para as unidades afetadas.
     */
    std::vector<Unit*> failedUnits;

    if (MassPath::consumeFailedMove(failedUnits)) {
        std::cout << "Rota impossível: unidades paradas.\n";

        for (Unit* u : failedUnits) {
            if (!u) {
                continue;
            }

            MassPath::clearPlan(u);

            u->setDestination(
                u->getPos(),
                              ptr_map->setGrid(),
                              ptr_units
            );
        }
    }

    /*
     * Déplacement des unités.
     */
    for (auto& u : ptr_units) {
        u->updateMove(
            ptr_map->setGrid(),
                      ptr_units,
                      TICK_RATE
        );
    }

    if (currentTick % TICK_RATE == 0)
    {
        // production de ressources, etc.
    }

    if (currentTick % (TICK_RATE * 5) == 0)
    {
        // événements rares
    }
}
