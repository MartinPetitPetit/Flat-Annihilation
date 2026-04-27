#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "map/map.hpp"
#include "display/display.hpp"
#include "startmenu/startmenu.hpp"

int main(int argc, char* argv[])
{
    const int MAP_W = 300;
    const int MAP_H = 300;
    Display_start_menu(argc, argv);
    MAP map = create_map(MAP_W, MAP_H);
    generate_map(map);
    DisplayMap(map, argc, argv, MAP_W, MAP_H);
    return EXIT_SUCCESS;
}

