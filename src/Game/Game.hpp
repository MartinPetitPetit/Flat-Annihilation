#pragma once

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
#include "../Backend/Unit/Unit.hpp"
#include "../Frontend/Sound/Sound.hpp"

static constexpr int   TICK_RATE    { 10   };
static constexpr float TICK_DELAY   { 1000.0f / TICK_RATE };
static constexpr int   FPS_CAP      { 160 };
static constexpr float FRAME_DELAY  { 1000.0f / FPS_CAP };

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
		std::unique_ptr<Map>              ptr_map;
		std::vector<std::unique_ptr<Player>> ptr_players;
		std::unique_ptr<Sound>            ptr_sound;

		// Toutes les unités de la scène
		std::vector<std::unique_ptr<Unit>> ptr_units;

		int    MAP_W   { 0     };
		int    MAP_H   { 0     };
		bool   running { false };
		float  productionAccumulator { 0.0f };
		int    hudFPS  { 0     };
		int    hudTPS  { 0     };
		Uint64 currentTick { 0 };
};