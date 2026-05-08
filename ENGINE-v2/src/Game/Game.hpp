// protection contre les appels multiples
#ifndef GAME_HPP
#define GAME_HPP



// ── bibliothèque ─────────────────────────────────────────────────────────────────
// entrée/sortie (scanf/printf)
#include <cstddef>
#include <iostream>
// allocation dynamique de matrice
#include <vector>
// allocation intelligente (unique_ptr)
#include <memory>

// ── fichier annexe ─────────────────────────────────────────────────────────────────

#include "../Player/Player.hpp"

// ── à retirer ─────────────────────────────────────────────────────────────────
class Window{};
class Renderer{};
class EventManager{};
class ResourceManager{};
class UIManager{};
class Map{public:Map(int maxX, int maxY){};};


// ── Game ─────────────────────────────────────────────────────────────────
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

		std::unique_ptr<Window> ptr_window;
		std::unique_ptr<Renderer> ptr_renderer;
		std::unique_ptr<EventManager> ptr_eventManager;
		std::unique_ptr<ResourceManager> ptr_resourceManager;
		std::unique_ptr<UIManager> ptr_uiManager;
		std::unique_ptr<Map> ptr_map;
		std::vector<std::unique_ptr<Player>> ptr_players;
		bool running = false;



	protected:

};


#endif