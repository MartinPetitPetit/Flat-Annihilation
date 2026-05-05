#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    int width;
    int height;
    bool fullscreen;
} DISPLAY_OPTIONS;



int Display_options_menu(int argc, char* argv[], DISPLAY_OPTIONS& options);
int Display_start_menu(int argc, char* argv[],DISPLAY_OPTIONS& options);