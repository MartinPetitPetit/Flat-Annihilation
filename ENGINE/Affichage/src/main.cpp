#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>

/*
#define X_Max 20
#define Y_Max 20

typedef enum {None_Struct,Usine,Production,Resource}STRUCTURE;
typedef enum {None_Resource,tree,gold,Sapling}RESOURCE;

typedef enum {None_Unit,archer,MONK}UNIT;



typedef struct MAP{
    CARRE tab[X_Max][Y_Max];
}MAP;
*/



template<typename T> constexpr T WIDTHSCREEN{ 800 };
template<typename T> constexpr T HEIGHTSCREEN{ 600 };

void get_text_and_rect(SDL_Renderer *renderer, int x, int y, const char *text,
        TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect);
void draw_case_fill(SDL_Renderer* renderer, const SDL_Rect& rectangle, const SDL_Color& color);
void draw_rectangle_not_fill(SDL_Renderer* renderer, const SDL_Rect& rectangle, const SDL_Color& color);


typedef enum {Plain,Montain,Lake,Bush,ravine}TERRAIN;// 0plain ;1 montain ;2 lake ,3 tree ; 4 gold
typedef enum { None1, Usine, Production, Resource } STRUCTURE;
typedef enum { None2, soldier, archer, MONK } UNIT;

typedef struct{
    TERRAIN type_terrain;
    STRUCTURE type_struct;
    UNIT type_unit;
}CARRE;

using MAP = std::vector<std::vector<CARRE>>;

/*MAP cree_map(int width, int height) {
    MAP map(width, std::vector<CARRE>(height));
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++) {
            map[x][y].type_terrain = rand() % 5;
            map[x][y].type_unit = None2;
            map[x][y].type_struct = None1;
        }
    return map;
}*/
int main(int argc, char* argv[])
{
    
    const int MAP_W = 3000;
    const int MAP_H = 3000;
    MAP map = cree_map(MAP_W, MAP_H);
    generate_map(map);


    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Init TTF
    if (TTF_Init() < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init: %s", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    char font_path[256];
    if (argc == 1) {
        strcpy(font_path, "src/FreeSans.ttf");
    } else if (argc == 2) {
        strcpy(font_path, argv[1]);
    } else {
        fprintf(stderr, "error: too many arguments\n");
        return EXIT_FAILURE;
    }

    TTF_Font *font = TTF_OpenFont(font_path, 24);
    if (font == NULL) {
        fprintf(stderr, "error: font not found: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Window* pWindow{ nullptr };
    SDL_Renderer* pRenderer{ nullptr };

    if (SDL_CreateWindowAndRenderer(WIDTHSCREEN<int>, HEIGHTSCREEN<int>,
        SDL_WINDOW_SHOWN, &pWindow, &pRenderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Event events;
    bool isOpen{ true };
    bool isBeingDragged{ false };

    int scale = 8;
    int offsetX = 0;
    int offsetY = 0;
    int dragStartX = 0;
    int dragStartY = 0;
    int dragStartOffsetX = 0;
    int dragStartOffsetY = 0;
    int selectedCellX = -1;
    int selectedCellY = -1;

    // Le tooltip est fixe : on crée la texture UNE SEULE FOIS, hors de la loop
    const char *tooltip = "Tooltip example";
    SDL_Texture *tooltipTexture = nullptr;
    SDL_Rect tooltipRect;
    get_text_and_rect(pRenderer, 0, 0, tooltip, font, &tooltipTexture, &tooltipRect);

    while (isOpen)
    {
        while (SDL_PollEvent(&events))
        {
            switch (events.type)
            {
            case SDL_QUIT:
                isOpen = false;
                break;

            case SDL_MOUSEWHEEL: {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                int oldScale = scale;
                if (events.wheel.y > 0 && scale < 64)
                    scale++;
                else if (events.wheel.y < 0 && scale > 2)
                    scale--;
                offsetX = mouseX - (mouseX - offsetX) * scale / oldScale;
                offsetY = mouseY - (mouseY - offsetY) * scale / oldScale;
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
                if (events.button.button == SDL_BUTTON_RIGHT) {
                    isBeingDragged = true;
                    dragStartX = events.button.x;
                    dragStartY = events.button.y;
                    dragStartOffsetX = offsetX;
                    dragStartOffsetY = offsetY;
                } else if (events.button.button == SDL_BUTTON_LEFT) {
                    int cx = (events.button.x - offsetX) / scale;
                    int cy = (events.button.y - offsetY) / scale;
                    // Vérifier que la cellule est dans les bornes
                    if (cx >= 0 && cx < MAP_W && cy >= 0 && cy < MAP_H) {
                        selectedCellX = cx;
                        selectedCellY = cy;
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (events.button.button == SDL_BUTTON_RIGHT)
                    isBeingDragged = false;
                break;

            case SDL_MOUSEMOTION:
                if (isBeingDragged) {
                    offsetX = dragStartOffsetX + (events.motion.x - dragStartX);
                    offsetY = dragStartOffsetY + (events.motion.y - dragStartY);
                }
                break;
            }
        }

        // --- Rendu ---
        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        SDL_Color color{ 255, 255, 255, 255 };

        for (int x = 0; x < MAP_W; x++) {
            for (int y = 0; y < MAP_H; y++) {
                SDL_Rect cell;
                cell.x = offsetX + x * scale;
                cell.y = offsetY + y * scale;
                cell.w = scale;
                cell.h = scale;

                if (cell.x + cell.w < 0 || cell.x > WIDTHSCREEN<int>) continue;
                if (cell.y + cell.h < 0 || cell.y > HEIGHTSCREEN<int>) continue;

                switch (map[x][y].type_terrain) {
                    case 0: color = { 0,   255, 0,   255 }; break;
                    case 1: color = { 139, 69,  19,  255 }; break;
                    case 2: color = { 255, 255, 0,   255 }; break;
                    case 3: color = { 100, 149, 237, 255 }; break;
                    case 4: color = { 0,   0,   255, 255 }; break;
                }
                draw_case_fill(pRenderer, cell, color);
            }
        }

        // Cellule sélectionnée
        if (selectedCellX >= 0 && selectedCellY >= 0) {
            SDL_Rect selectedCell;
            selectedCell.x = offsetX + selectedCellX * scale;
            selectedCell.y = offsetY + selectedCellY * scale;
            selectedCell.w = scale;
            selectedCell.h = scale;
            draw_case_fill(pRenderer, selectedCell, { 255, 0, 0, 255 });
        }

        // Texte — rendu après la carte, avant RenderPresent
        SDL_RenderCopy(pRenderer, tooltipTexture, NULL, &tooltipRect);

        // Un seul RenderPresent par frame
        SDL_RenderPresent(pRenderer);
    }

    // Libération
    SDL_DestroyTexture(tooltipTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
    return EXIT_SUCCESS;
}

void draw_case_fill(SDL_Renderer* renderer, const SDL_Rect& rectangle, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rectangle);
}

void draw_rectangle_not_fill(SDL_Renderer* renderer, const SDL_Rect& rectangle, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rectangle); // SDL_RenderDrawRect remplace la boucle manuelle
}

void get_text_and_rect(SDL_Renderer *renderer, int x, int y, const char *text,
        TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect) {
    SDL_Color textColor = {255, 255, 255, 255}; // Alpha 255, pas 0
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, textColor);
    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect->x = x;
    rect->y = y;
    rect->w = surface->w;
    rect->h = surface->h;
    SDL_FreeSurface(surface);
}
////////////----------------------------------------------------------------------------------------------------------


MAP cree_map(int width, int height)
{
    MAP map(width, std::vector<CARRE>(height));

    return map;
}

void generate_map(MAP& map)
{
    static bool srand_already_called = false;

    if (!srand_already_called) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        srand_already_called = true;
    }

    int width = static_cast<int>(map.size());

    if (width == 0) {
        return;
    }

    int height = static_cast<int>(map[0].size());

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            map[x][y].type_terrain = static_cast<TERRAIN>(std::rand() % 5);
            map[x][y].type_struct = None1;
            map[x][y].type_unit = None2;
        }
    }
}

void affiche_map(const MAP& map)
{
    int width = static_cast<int>(map.size());

    if (width == 0) {
        return;
    }

    int height = static_cast<int>(map[0].size());

    for (int x = 0; x < width; x++) {
        std::cout << "|";
        for (int y = 0; y < height; y++) {
            std::cout << " " << map[x][y].type_terrain << " |";
        }
        std::cout << std::endl;
    }
}

int rac_TERRAIN_size_rec(
    const MAP& map,
    int x,
    int y,
    TERRAIN type_,
    std::vector<std::vector<int>>& visited
)
{
    int width = static_cast<int>(map.size());

    if (width == 0) {
        return 0;
    }

    int height = static_cast<int>(map[0].size());

    if (x < 0 || x >= width || y < 0 || y >= height) {
        return 0;
    }

    if (visited[x][y] == 1) {
        return 0;
    }

    if (map[x][y].type_terrain != type_) {
        return 0;
    }

    visited[x][y] = 1;

    int total = 1;

    total += rac_TERRAIN_size_rec(map, x + 1, y,     type_, visited);
    total += rac_TERRAIN_size_rec(map, x - 1, y,     type_, visited);
    total += rac_TERRAIN_size_rec(map, x,     y + 1, type_, visited);
    total += rac_TERRAIN_size_rec(map, x,     y - 1, type_, visited);

    total += rac_TERRAIN_size_rec(map, x + 1, y + 1, type_, visited);
    total += rac_TERRAIN_size_rec(map, x + 1, y - 1, type_, visited);
    total += rac_TERRAIN_size_rec(map, x - 1, y + 1, type_, visited);
    total += rac_TERRAIN_size_rec(map, x - 1, y - 1, type_, visited);

    return total;
}

int terrain_size(const MAP& map, int x, int y)
{
    int width = static_cast<int>(map.size());

    if (width == 0) {
        return 0;
    }

    int height = static_cast<int>(map[0].size());

    if (x < 0 || x >= width || y < 0 || y >= height) {
        return 0;
    }

    std::vector<std::vector<int>> visited(
        width,
        std::vector<int>(height, 0)
    );

    TERRAIN terrain_atual = map[x][y].type_terrain;

    return rac_TERRAIN_size_rec(map, x, y, terrain_atual, visited);
}
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>

/*
#define X_Max 20
#define Y_Max 20
typedef enum {Plain,Montain,Lake,Bush,ravine}TERRAIN;// 0plain ;1 montain ;2 lake ,3 tree ; 4 gold
typedef enum {None_Struct,Usine,Production,Resource}STRUCTURE;
typedef enum {None_Resource,tree,gold,Sapling}RESOURCE;

typedef enum {None_Unit,archer,MONK}UNIT;



typedef struct MAP{
    CARRE tab[X_Max][Y_Max];
}MAP;
*/



template<typename T> constexpr T WIDTHSCREEN{ 800 };
template<typename T> constexpr T HEIGHTSCREEN{ 600 };

void get_text_and_rect(SDL_Renderer *renderer, int x, int y, const char *text,
        TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect);
void draw_case_fill(SDL_Renderer* renderer, const SDL_Rect& rectangle, const SDL_Color& color);
void draw_rectangle_not_fill(SDL_Renderer* renderer, const SDL_Rect& rectangle, const SDL_Color& color);



typedef enum { None1, Usine, Production, Resource } STRUCTURE;
typedef enum { None2, soldier, archer, MONK } UNIT;

typedef struct{
    TERRAIN type_terrain;
    STRUCTURE type_struct;
    UNIT type_unit;
}CARRE;

using MAP = std::vector<std::vector<CARRE>>;

MAP cree_map(int width, int height) {
    MAP map(width, std::vector<CARRE>(height));
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++) {
            map[x][y].type_terrain = rand() % 5;
            map[x][y].type_unit = None2;
            map[x][y].type_struct = None1;
        }
    return map;
}
int main(int argc, char* argv[])
{
    
    const int MAP_W = 3000;
    const int MAP_H = 3000;
    MAP map = cree_map(MAP_W, MAP_H);


    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Init TTF
    if (TTF_Init() < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init: %s", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    char font_path[256];
    if (argc == 1) {
        strcpy(font_path, "src/FreeSans.ttf");
    } else if (argc == 2) {
        strcpy(font_path, argv[1]);
    } else {
        fprintf(stderr, "error: too many arguments\n");
        return EXIT_FAILURE;
    }

    TTF_Font *font = TTF_OpenFont(font_path, 24);
    if (font == NULL) {
        fprintf(stderr, "error: font not found: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Window* pWindow{ nullptr };
    SDL_Renderer* pRenderer{ nullptr };

    if (SDL_CreateWindowAndRenderer(WIDTHSCREEN<int>, HEIGHTSCREEN<int>,
        SDL_WINDOW_SHOWN, &pWindow, &pRenderer) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Event events;
    bool isOpen{ true };
    bool isBeingDragged{ false };

    int scale = 8;
    int offsetX = 0;
    int offsetY = 0;
    int dragStartX = 0;
    int dragStartY = 0;
    int dragStartOffsetX = 0;
    int dragStartOffsetY = 0;
    int selectedCellX = -1;
    int selectedCellY = -1;

    // Le tooltip est fixe : on crée la texture UNE SEULE FOIS, hors de la loop
    const char *tooltip = "Tooltip example";
    SDL_Texture *tooltipTexture = nullptr;
    SDL_Rect tooltipRect;
    get_text_and_rect(pRenderer, 0, 0, tooltip, font, &tooltipTexture, &tooltipRect);

    while (isOpen)
    {
        while (SDL_PollEvent(&events))
        {
            switch (events.type)
            {
            case SDL_QUIT:
                isOpen = false;
                break;

            case SDL_MOUSEWHEEL: {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                int oldScale = scale;
                if (events.wheel.y > 0 && scale < 64)
                    scale++;
                else if (events.wheel.y < 0 && scale > 2)
                    scale--;
                offsetX = mouseX - (mouseX - offsetX) * scale / oldScale;
                offsetY = mouseY - (mouseY - offsetY) * scale / oldScale;
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
                if (events.button.button == SDL_BUTTON_RIGHT) {
                    isBeingDragged = true;
                    dragStartX = events.button.x;
                    dragStartY = events.button.y;
                    dragStartOffsetX = offsetX;
                    dragStartOffsetY = offsetY;
                } else if (events.button.button == SDL_BUTTON_LEFT) {
                    int cx = (events.button.x - offsetX) / scale;
                    int cy = (events.button.y - offsetY) / scale;
                    // Vérifier que la cellule est dans les bornes
                    if (cx >= 0 && cx < MAP_W && cy >= 0 && cy < MAP_H) {
                        selectedCellX = cx;
                        selectedCellY = cy;
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (events.button.button == SDL_BUTTON_RIGHT)
                    isBeingDragged = false;
                break;

            case SDL_MOUSEMOTION:
                if (isBeingDragged) {
                    offsetX = dragStartOffsetX + (events.motion.x - dragStartX);
                    offsetY = dragStartOffsetY + (events.motion.y - dragStartY);
                }
                break;
            }
        }

        // --- Rendu ---
        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        SDL_Color color{ 255, 255, 255, 255 };

        for (int x = 0; x < MAP_W; x++) {
            for (int y = 0; y < MAP_H; y++) {
                SDL_Rect cell;
                cell.x = offsetX + x * scale;
                cell.y = offsetY + y * scale;
                cell.w = scale;
                cell.h = scale;

                if (cell.x + cell.w < 0 || cell.x > WIDTHSCREEN<int>) continue;
                if (cell.y + cell.h < 0 || cell.y > HEIGHTSCREEN<int>) continue;

                switch (map[x][y].type_terrain) {
                    case 0: color = { 0,   255, 0,   255 }; break;
                    case 1: color = { 139, 69,  19,  255 }; break;
                    case 2: color = { 255, 255, 0,   255 }; break;
                    case 3: color = { 100, 149, 237, 255 }; break;
                    case 4: color = { 0,   0,   255, 255 }; break;
                }
                draw_case_fill(pRenderer, cell, color);
            }
        }

        // Cellule sélectionnée
        if (selectedCellX >= 0 && selectedCellY >= 0) {
            SDL_Rect selectedCell;
            selectedCell.x = offsetX + selectedCellX * scale;
            selectedCell.y = offsetY + selectedCellY * scale;
            selectedCell.w = scale;
            selectedCell.h = scale;
            draw_case_fill(pRenderer, selectedCell, { 255, 0, 0, 255 });
        }

        // Texte — rendu après la carte, avant RenderPresent
        SDL_RenderCopy(pRenderer, tooltipTexture, NULL, &tooltipRect);

        // Un seul RenderPresent par frame
        SDL_RenderPresent(pRenderer);
    }

    // Libération
    SDL_DestroyTexture(tooltipTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
    return EXIT_SUCCESS;
}

void draw_case_fill(SDL_Renderer* renderer, const SDL_Rect& rectangle, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rectangle);
}

void draw_rectangle_not_fill(SDL_Renderer* renderer, const SDL_Rect& rectangle, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rectangle); // SDL_RenderDrawRect remplace la boucle manuelle
}

void get_text_and_rect(SDL_Renderer *renderer, int x, int y, const char *text,
        TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect) {
    SDL_Color textColor = {255, 255, 255, 255}; // Alpha 255, pas 0
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, textColor);
    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect->x = x;
    rect->y = y;
    rect->w = surface->w;
    rect->h = surface->h;
    SDL_FreeSurface(surface);
}
////////////----------------------------------------------------------------------------------------------------------


MAP cree_map(int width, int height)
{
    MAP map(width, std::vector<CARRE>(height));

    return map;
}

void generate_map(MAP& map)
{
    static bool srand_already_called = false;

    if (!srand_already_called) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        srand_already_called = true;
    }

    int width = static_cast<int>(map.size());

    if (width == 0) {
        return;
    }

    int height = static_cast<int>(map[0].size());

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            map[x][y].type_terrain = static_cast<TERRAIN>(std::rand() % 5);
            map[x][y].type_struct = None1;
            map[x][y].type_unit = None2;
        }
    }
}

void affiche_map(const MAP& map)
{
    int width = static_cast<int>(map.size());

    if (width == 0) {
        return;
    }

    int height = static_cast<int>(map[0].size());

    for (int x = 0; x < width; x++) {
        std::cout << "|";
        for (int y = 0; y < height; y++) {
            std::cout << " " << map[x][y].type_terrain << " |";
        }
        std::cout << std::endl;
    }
}

int rac_TERRAIN_size_rec(
    const MAP& map,
    int x,
    int y,
    TERRAIN type_,
    std::vector<std::vector<int>>& visited
)
{
    int width = static_cast<int>(map.size());

    if (width == 0) {
        return 0;
    }

    int height = static_cast<int>(map[0].size());

    if (x < 0 || x >= width || y < 0 || y >= height) {
        return 0;
    }

    if (visited[x][y] == 1) {
        return 0;
    }

    if (map[x][y].type_terrain != type_) {
        return 0;
    }

    visited[x][y] = 1;

    int total = 1;

    total += rac_TERRAIN_size_rec(map, x + 1, y,     type_, visited);
    total += rac_TERRAIN_size_rec(map, x - 1, y,     type_, visited);
    total += rac_TERRAIN_size_rec(map, x,     y + 1, type_, visited);
    total += rac_TERRAIN_size_rec(map, x,     y - 1, type_, visited);

    total += rac_TERRAIN_size_rec(map, x + 1, y + 1, type_, visited);
    total += rac_TERRAIN_size_rec(map, x + 1, y - 1, type_, visited);
    total += rac_TERRAIN_size_rec(map, x - 1, y + 1, type_, visited);
    total += rac_TERRAIN_size_rec(map, x - 1, y - 1, type_, visited);

    return total;
}

int terrain_size(const MAP& map, int x, int y)
{
    int width = static_cast<int>(map.size());

    if (width == 0) {
        return 0;
    }

    int height = static_cast<int>(map[0].size());

    if (x < 0 || x >= width || y < 0 || y >= height) {
        return 0;
    }

    std::vector<std::vector<int>> visited(
        width,
        std::vector<int>(height, 0)
    );

    TERRAIN terrain_atual = map[x][y].type_terrain;

    return rac_TERRAIN_size_rec(map, x, y, terrain_atual, visited);
}
