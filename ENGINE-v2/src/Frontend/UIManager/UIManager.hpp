#pragma once
#include "../Renderer/Renderer.hpp"
#include "../SelectionManager/SelectionManager.hpp"
#include "../../Backend/Map/Map.hpp"
#include "../Sound/Sound.hpp"

class UIManager {
    Renderer*         renderer { nullptr };
public:
    UIManager(Renderer& renderer);

    void renderHUD();
    void renderMinimap(MAP& map, int MAP_W, int MAP_H);
    void renderSelectionPanel();
    void renderDragRect(SelectionManager& sel);

    // Menu principal
    int  showMainMenu(DISPLAY_OPTIONS& options,Sound& sound);
    void showOptionsMenu(DISPLAY_OPTIONS& options);

private:
    // Helpers menu
    void drawButton(const char* label, SDL_Rect rect, bool selected);
    SDL_Texture* makeTextTexture(const char* text, SDL_Color color);
};