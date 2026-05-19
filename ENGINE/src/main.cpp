#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "map/map.hpp"
#include "display/display.hpp"
#include "startmenu/startmenu.hpp"

int main(int argc, char* argv[])
{
    int MAP_W =300;
    int MAP_H = 300;
    /*
    int MAP_W, MAP_H;
    std::cout << "MAP_W = ? ";
    std::cin >> MAP_W;
    std::cout << "MAP_H = ? ";
    std::cin >> MAP_H;
    */
    DISPLAY_OPTIONS options = { DEFAULT_WIDTH, DEFAULT_HEIGHT, false };
    MAP map = create_map(MAP_W, MAP_H);
    int choice =0;
    while (choice != -1) {
        choice = Display_start_menu(argc, argv,options);
        switch (choice) {
            case 0:
                generate_map(map);
                std::cout << "Map generated." << std::endl;
                break;
            case 1:
                DisplayMap(map, argc, argv, MAP_W, MAP_H,options);
                std::cout << "Now playing solo vs IA." << std::endl;
                break;
            case 2:
                DisplayMap(map, argc, argv, MAP_W, MAP_H,options);
                std::cout << "Now playing multiplayer." << std::endl;
                break;
            case 3:
                Display_options_menu(argc, argv, options);
                std::cout << "Game options" << std::endl;
                break;
            case -1:
                std::cout << "Quitting game." << std::endl;
                return EXIT_SUCCESS;
            default:
                return EXIT_SUCCESS;
            }
        }
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
}

