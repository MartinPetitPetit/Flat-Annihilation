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

    this->ptr_map = std::make_unique<MAP>(create_map(MAP_W, MAP_H));
    generate_map(*ptr_map);

    // Joueur humain
    this->ptr_players.push_back(std::make_unique<Player>());

    // Joueurs IA
    int nbIA = 0;
    std::cout << "combien de joueurs IA ? ";
    std::cin >> nbIA;
    for (int i = 0; i < nbIA; i++)
        this->ptr_players.push_back(std::make_unique<Player>(i));

    for (auto& p : ptr_players)
        std::cout << "joueur : " << p->getName() << "\n";

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
    while (running && !ptr_eventManager->isQuit())
    {
        this->ptr_eventManager->pollEvents();
        this->update();

        this->ptr_renderer->clear();
        this->ptr_renderer->drawMap(*ptr_map, MAP_W, MAP_H, options);
        this->ptr_uiManager->renderDragRect(*ptr_selectionManager);
        this->ptr_uiManager->renderHUD();
        this->ptr_renderer->present();
    }
}

void Game::update()
{
    // TODO : logique de jeu (tours, IA, etc.)
}