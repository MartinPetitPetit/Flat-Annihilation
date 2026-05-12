#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <vector>
#include <memory>

#include "../Frontend/Window/Window.hpp"
#include "../Frontend/Renderer/Renderer.hpp"
#include "../Frontend/EventManager/EventManager.hpp"
#include "../Frontend/SelectionManager/SelectionManager.hpp"
#include "../Frontend/UIManager/UIManager.hpp"
#include "../Backend/Map/Map.hpp"
#include "../Backend/Player/Player.hpp"

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
    DISPLAY_OPTIONS  options { 800, 600, false };

    std::unique_ptr<Window>           ptr_window;
    std::unique_ptr<Renderer>         ptr_renderer;
    std::unique_ptr<SelectionManager> ptr_selectionManager;
    std::unique_ptr<EventManager>     ptr_eventManager;
    std::unique_ptr<UIManager>        ptr_uiManager;
    std::unique_ptr<MAP>              ptr_map;
    std::vector<std::unique_ptr<Player>> ptr_players;

    int  MAP_W   { 0     };
    int  MAP_H   { 0     };
    bool running { false };
};

#endif