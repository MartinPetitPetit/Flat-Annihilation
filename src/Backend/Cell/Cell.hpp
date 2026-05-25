#pragma once

#include <SDL2/SDL_image.h>

#include "../Resource/Resource.hpp"




/*
 * ============================================================
 * forward declaration
 * ============================================================
 */

class Unit;

/*
 * ============================================================
 * TERRAIN LAYER
 * ============================================================
 */

// Ground layer.
enum TERRAIN {
	Plain,      // Basic terrain.
	Montain,   // Mountain terrain.
	Lake,      // Lake water.
	River,     // River water.
	Bush,      // Legacy terrain bush. Prefer RESOURCE::BushResource.
	ravine     // Cracked terrain.
};

/*
 * ============================================================
 * STRUCTURE LAYER
 * ============================================================
 */

// Building layer.
enum STRUCTURE {
	None_Struct,
	Usine,
	Production,
};

/*
 * ============================================================
 * CELL
 * ============================================================
 */

class Cell
{
	public:

		Cell();
		virtual ~Cell();

		TERRAIN type_terrain;
		STRUCTURE type_struct;
		Resource *resource;
		Unit *unit;

		bool has_berry;

		bool walkable;
		bool occupied;
};