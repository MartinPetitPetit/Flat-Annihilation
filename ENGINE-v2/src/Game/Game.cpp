#include "Game.hpp"
#include <iostream>

Game::Game()
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    ptr_window   = std::make_unique<Window>("Flat Annihilation", options);
    ptr_renderer = std::make_unique<Renderer>(*ptr_window, "Starjedi.ttf");
    ptr_uiManager = std::make_unique<UIManager>(*ptr_renderer);
    ptr_selectionManager = std::make_unique<SelectionManager>();
    ptr_eventManager = std::make_unique<EventManager>(*ptr_selectionManager, *ptr_renderer);
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
    // Menu principal
    int choice = ptr_uiManager->showMainMenu(options);
    if (choice < 0) return;

    // Taille de la carte
    std::cout << "taille de la carte : MAP_W MAP_H = ";
    std::cin >> MAP_W >> MAP_H;

    ptr_map = std::make_unique<MAP>(create_map(MAP_W, MAP_H));
    generate_map(*ptr_map);

    // Joueur humain
    ptr_players.push_back(std::make_unique<Player>());

    // Joueurs IA
    int nbIA = 0;
    std::cout << "combien de joueurs IA ? ";
    std::cin >> nbIA;
    for (int i = 0; i < nbIA; i++)
        ptr_players.push_back(std::make_unique<Player>(i));

    for (auto& p : ptr_players)
        std::cout << "joueur : " << p->getName() << "\n";

    running = true;
    run();
}

void Game::stopGame()
{
    ptr_players.clear();
    ptr_map.reset();
    running = false;
}

void Game::run()
{
    Uint32 lastTick       = SDL_GetTicks();
    Uint32 lastFrame      = SDL_GetTicks();
    Uint32 lastStatsTime  = SDL_GetTicks(); // ← timer pour les stats
    float  tickAccumulator = 0.0f;
    int    frameCount     = 0;
    int    tickCount      = 0;

    while (running && !ptr_eventManager->isQuit())
    {
        Uint32 now     = SDL_GetTicks();
        float  elapsed = static_cast<float>(now - lastTick);
        lastTick       = now;

        tickAccumulator += elapsed;

        ptr_eventManager->pollEvents();

        while (tickAccumulator >= TICK_DELAY)
        {
            update();
            tickAccumulator -= TICK_DELAY;
            tickCount++;  // ← compter les ticks

            if (tickAccumulator > TICK_DELAY * 5) {
                tickAccumulator = 0;
                break;
            }
        }

        Uint32 frameNow = SDL_GetTicks();
        if (FPS_CAP == 0 || static_cast<float>(frameNow - lastFrame) >= FRAME_DELAY)
        {
            ptr_renderer->clear();
            ptr_renderer->drawMap(*ptr_map, MAP_W, MAP_H, options);
            ptr_uiManager->renderDragRect(*ptr_selectionManager);
            ptr_uiManager->renderHUD();
            ptr_renderer->present();
            lastFrame = frameNow;
            frameCount++;  // ← compter les frames
        }
        else
        {
            SDL_Delay(1);
        }

        // -- Affichage des stats toutes les secondes
        Uint32 statsNow = SDL_GetTicks();
        if (statsNow - lastStatsTime >= 1000)
        {
            std::cout << "FPS: " << frameCount
                      << " | TPS: " << tickCount
                      << " | cible: " << TICK_RATE << " ticks/s"
                      << "\r" << std::flush; // \r écrase la ligne précédente
            frameCount    = 0;
            tickCount     = 0;
            lastStatsTime = statsNow;
        }
    }

    std::cout << "\n"; // saut de ligne propre à la fin
}

void Game::update()
{
    // TODO : logique de jeu (tours, IA, etc.)
}