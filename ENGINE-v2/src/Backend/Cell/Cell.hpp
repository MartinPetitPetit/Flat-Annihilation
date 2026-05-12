#pragma once

#include <SDL2/SDL_image.h>

enum cellType {
    // Plain,      // Basic walkable terrain.
    // Montain,   // Mountain terrain.
    // Lake,      // Lake water.
    // River,     // River water.
    // ravine     // Cracked or torn terrain.
};


class Cell
{

	public:
		Cell();
		virtual ~Cell();

		cellType type;
		bool walkable;
		bool occupied;

		SDL_Texture *texture;

	private:




	protected:
	
};