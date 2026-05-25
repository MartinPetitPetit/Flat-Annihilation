#pragma once
#include "../Renderer/Renderer.hpp"
#include "../SelectionManager/SelectionManager.hpp"
#include "../../Backend/Map/Map.hpp"
#include "../Sound/Sound.hpp"

class UIManager {
    Renderer* renderer { nullptr };
    Window*   window   { nullptr };

    // État HUD
    bool  gamePaused  { false };
    int   currentFPS  { 0     };
    int   currentTPS  { 0     };
    Uint64 gameTick   { 0     };
    int   tickRate    { 10    };

public:
    UIManager(Renderer& renderer, Window& window);

    // Mise à jour des stats affichées dans le HUD
    void setHUDStats(int fps, int tps, Uint64 tick, int tickRateVal);
    bool isGamePaused() const;

    void renderHUD();
    void renderMinimap(MAP& map, int MAP_W, int MAP_H);
    void renderSelectionPanel();
    void renderDragRect(SelectionManager& sel);

    // Gestion des clics HUD (retourne true si le clic a été consommé)
    bool handleHUDClick(int x, int y);

    // Menu principal
    int  showMainMenu(DISPLAY_OPTIONS& options, Sound& sound);
    void showOptionsMenu(DISPLAY_OPTIONS& options, Sound& sound);

private:
    // Helpers menu
    void drawButton(const char* label, SDL_Rect rect, bool selected, bool disabled);
    void applyResolution(DISPLAY_OPTIONS& options, int w, int h, bool fullscreen);
    SDL_Texture* makeTextTexture(const char* text, SDL_Color color);

    // Helpers HUD
    void drawHUDRect(SDL_Rect r, SDL_Color fill, SDL_Color border);
    void drawHUDText(const char* text, int x, int y, SDL_Color color);
    SDL_Rect getHUDPauseRect()     const;
    SDL_Rect getHUDStopRect()      const;
    SDL_Rect getHUDSelectionRect() const;
    SDL_Rect getHUDBuildingRect()  const;
};