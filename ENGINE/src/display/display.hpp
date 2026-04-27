#include "../map/map.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


void get_text_and_rect(SDL_Renderer *renderer, int x, int y, const char *text,
        TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect);
void draw_case_fill(SDL_Renderer* renderer, const SDL_Rect& rectangle, const SDL_Color& color);
void draw_rectangle_not_fill(SDL_Renderer* renderer, const SDL_Rect& rectangle, const SDL_Color& color);
int DisplayMap(MAP& map, int argc, char* argv[],int MAP_W, int MAP_H);