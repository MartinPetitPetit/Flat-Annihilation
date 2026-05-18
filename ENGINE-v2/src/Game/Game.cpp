#include "Game.hpp"
#include <iostream>
#include <vector>
#include <algorithm>

Game::Game()
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

	ptr_sound = std::make_unique<Sound>();
	ptr_sound->load("click",  "sounds/rhoo.wav");
	ptr_sound->load("hover",  "sounds/ptiou.wav");	

    ptr_window   = std::make_unique<Window>("Flat Annihilation", options);
    ptr_renderer = std::make_unique<Renderer>(*ptr_window, "Starjedi.ttf");
    ptr_uiManager = std::make_unique<UIManager>(*ptr_renderer);
    ptr_selectionManager = std::make_unique<SelectionManager>();
    ptr_eventManager = std::make_unique<EventManager>(*ptr_selectionManager, *ptr_renderer);
    ptr_unitManager = std::make_unique<UnitManager>();
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

struct SpawnPoint
{
    int x;
    int y;
};

static bool is_valid_base_cell(const MAP& map, int x, int y)
{
    if (x < 0 || y < 0)
        return false;

    if (x >= static_cast<int>(map.size()))
        return false;

    if (y >= static_cast<int>(map[0].size()))
        return false;

    const Cell& cell = map[x][y];

    if (cell.type_terrain != Plain)
        return false;

    if (cell.type_struct != None_Struct)
        return false;

    if (cell.type_unit != None_Unit)
        return false;

    if (cell.type_resource != None_Resource)
        return false;

    if (cell.occupied)
        return false;

    return true;
}

static bool find_valid_base_position_near(
    const MAP& map,
    int target_x,
    int target_y,
    int& out_x,
    int& out_y
)
{
    int width = static_cast<int>(map.size());

    if (width == 0)
        return false;

    int height = static_cast<int>(map[0].size());

    int max_radius = std::max(width, height);

    for (int radius = 0; radius < max_radius; radius++)
    {
        for (int dx = -radius; dx <= radius; dx++)
        {
            for (int dy = -radius; dy <= radius; dy++)
            {
                bool border =
                dx == -radius ||
                dx ==  radius ||
                dy == -radius ||
                dy ==  radius;

                if (!border)
                    continue;

                int x = target_x + dx;
                int y = target_y + dy;

                if (is_valid_base_cell(map, x, y))
                {
                    out_x = x;
                    out_y = y;
                    return true;
                }
            }
        }
    }

    return false;
}

static std::vector<SpawnPoint> build_spawn_targets(
    int width,
    int height,
    int human_count,
    int ai_count
)
{
    std::vector<SpawnPoint> targets;

    int margin_x = std::max(3, width / 10);
    int margin_y = std::max(3, height / 10);

    int center_x = width / 2;
    int center_y = height / 2;

    /*
     * Human players first.
     *
     * For 2 human players:
     * player 0 -> left side
     * player 1 -> right side
     */
    if (human_count >= 1)
        targets.push_back({ margin_x, center_y });

    if (human_count >= 2)
        targets.push_back({ width - 1 - margin_x, center_y });

    /*
     * If there are more than 2 human players,
     * distribute them on top and bottom sides.
     */
    if (human_count >= 3)
        targets.push_back({ center_x, margin_y });

    if (human_count >= 4)
        targets.push_back({ center_x, height - 1 - margin_y });

    /*
     * Extra human players, if any, go around corners.
     */
    for (int i = 4; i < human_count; i++)
    {
        int slot = i - 4;

        switch (slot % 4)
        {
            case 0:
                targets.push_back({ margin_x, margin_y });
                break;

            case 1:
                targets.push_back({ width - 1 - margin_x, margin_y });
                break;

            case 2:
                targets.push_back({ margin_x, height - 1 - margin_y });
                break;

            default:
                targets.push_back({ width - 1 - margin_x, height - 1 - margin_y });
                break;
        }
    }

    /*
     * AI players after humans.
     * They use remaining corners/sides.
     */
    for (int i = 0; i < ai_count; i++)
    {
        switch (i % 8)
        {
            case 0:
                targets.push_back({ margin_x, margin_y });
                break;

            case 1:
                targets.push_back({ width - 1 - margin_x, height - 1 - margin_y });
                break;

            case 2:
                targets.push_back({ width - 1 - margin_x, margin_y });
                break;

            case 3:
                targets.push_back({ margin_x, height - 1 - margin_y });
                break;

            case 4:
                targets.push_back({ center_x, margin_y });
                break;

            case 5:
                targets.push_back({ center_x, height - 1 - margin_y });
                break;

            case 6:
                targets.push_back({ margin_x, center_y });
                break;

            default:
                targets.push_back({ width - 1 - margin_x, center_y });
                break;
        }
    }

    return targets;
}

void Game::startGame()
{
    // Menu principal

	int choice = ptr_uiManager->showMainMenu(options, *ptr_sound);
    if (choice < 0) return;

    if (choice < 0)
        return;

    /*
     * Map size.
     */
    std::cout << "taille de la carte : MAP_W MAP_H = ";
    std::cin >> MAP_W >> MAP_H;

    /*
     * Number of players.
     */
    int human_count = 0;
    int ai_count = 0;

    std::cout << "nombre de joueurs humains : ";
    std::cin >> human_count;

    std::cout << "nombre de joueurs IA : ";
    std::cin >> ai_count;

    if (human_count < 0)
        human_count = 0;

    if (ai_count < 0)
        ai_count = 0;

    int total_players = human_count + ai_count;

    if (total_players <= 0)
    {
        std::cout << "Aucun joueur. Création d'un joueur humain par défaut.\n";
        human_count = 1;
        total_players = 1;
    }

    /*
     * Map creation.
     */
    this->ptr_map = std::make_unique<MAP>(create_map(MAP_W, MAP_H));
    generate_map(*ptr_map);

    /*
     * Unit manager.
     */
    if (!ptr_unitManager)
    {
        ptr_unitManager = std::make_unique<UnitManager>();
    }

    /*
     * Create human players.
     */
    for (int i = 0; i < human_count; i++)
    {
        std::cout << "\nJoueur humain " << i << "\n";
        this->ptr_players.push_back(std::make_unique<Player>());
    }

    /*
     * Create AI players.
     */
    for (int i = 0; i < ai_count; i++)
    {
        this->ptr_players.push_back(std::make_unique<Player>(i));
    }

    /*
     * Spawn bases.
     */
    std::vector<SpawnPoint> spawn_targets = build_spawn_targets(
        MAP_W,
        MAP_H,
        human_count,
        ai_count
    );

    for (int player_id = 0; player_id < total_players; player_id++)
    {
        int target_x = spawn_targets[player_id].x;
        int target_y = spawn_targets[player_id].y;

        int base_x = target_x;
        int base_y = target_y;

        bool found = find_valid_base_position_near(
            *ptr_map,
            target_x,
            target_y,
            base_x,
            base_y
        );

        if (!found)
        {
            std::cout
            << "Impossible de trouver une position valide pour la base du joueur "
            << player_id
            << "\n";

            continue;
        }

        bool ok = create_player_base_with_collectors(
            *ptr_map,
            *ptr_unitManager,
            base_x,
            base_y,
            player_id,
            5
        );

        if (ok)
        {
            std::cout
            << "Base du joueur "
            << player_id
            << " créée en ("
            << base_x
            << ", "
            << base_y
            << ") avec 5 collecteurs.\n";
        }
        else
        {
            std::cout
            << "Base du joueur "
            << player_id
            << " créée, mais les 5 collecteurs n'ont pas tous été créés.\n";
        }
    }

    /*
     * Start game loop.
     */
    running = true;
    run();
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

        this->ptr_eventManager->pollEvents();

        while (tickAccumulator >= TICK_DELAY)
        {
            this->update();
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
        this->ptr_renderer->clear();
        this->ptr_renderer->drawMap(*ptr_map, MAP_W, MAP_H, options);
        this->ptr_uiManager->renderDragRect(*ptr_selectionManager);
        this->ptr_uiManager->renderHUD();
        this->ptr_renderer->present();
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
				float gameTimeSeconds = static_cast<float>(currentTick) / TICK_RATE;
				int   minutes         = static_cast<int>(gameTimeSeconds) / 60;
				int   seconds         = static_cast<int>(gameTimeSeconds) % 60;

				std::cout << "FPS: "   << frameCount
						<< " | TPS: " << tickCount
						<< " | tick: " << currentTick
						<< " | temps: " << minutes << "m" << seconds << "s"
						<< "\r" << std::flush;

				frameCount    = 0;
				tickCount     = 0;
				lastStatsTime = statsNow;
			}
    }

    std::cout << "\n"; // saut de ligne propre à la fin
}

void Game::update()
{
    currentTick++;

    // -- Chaque tick : déplacements, collisions, etc.
    // ptr_map->update(currentTick);  // TODO

    // -- Toutes les secondes de jeu (TICK_RATE ticks)
    if (currentTick % TICK_RATE == 0)
    {
        // production de ressources, etc.
    }

    // -- Toutes les 5 secondes
    if (currentTick % (TICK_RATE * 5) == 0)
    {
        // événements rares
    }

    // -- Convertir en secondes de jeu si besoin
    // float gameTimeSeconds = static_cast<float>(currentTick) / TICK_RATE;
}
void Game::stopGame()
{
    running = false;

    ptr_players.clear();

    ptr_map.reset();

    if (ptr_unitManager) {
        ptr_unitManager.reset();
    }

    std::cout << "Game stopped.\n";
}
