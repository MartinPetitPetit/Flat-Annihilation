#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "map/map.hpp"
#include "display/display.hpp"






int main(int argc, char* argv[])
{
    const int MAP_W = 3000;
    const int MAP_H = 3000;
    MAP map = create_map(MAP_W, MAP_H);
    generate_map(map);
    return DisplayMap(map, argc, argv, MAP_W, MAP_H);
}

