/*
 * Frontend/Renderer/Renderer.hpp
 *
 * Rôle du fichier :
 * Declares the renderer API used to draw textures, text, map cells, shapes, and access SDL renderer resources.
 *
 * Notes de lecture :
 * Ce module centralise le rendu SDL afin que le reste du frontend n'appelle pas directement toutes les primitives bas niveau.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include "../Window/Window.hpp"
#include "../../Backend/Map/Map.hpp"


/*
 * Wrapper SDL chargé du dessin 2D du jeu.
 * Il conserve le renderer SDL, la police et les paramètres de caméra.
 */
class Renderer {
		/* Méthodes appelées par le reste du frontend pour dessiner et contrôler la vue. */
public:
		Renderer(Window& window, const char* font_path);
		~Renderer();

		void clear();
		void present();

		void drawTexture(SDL_Texture* tex, SDL_Rect* src, SDL_Rect* dst);
		void drawRect(const SDL_Rect& rect, SDL_Color color, bool filled);
		void drawFilledCircle(int cx, int cy, int radius, SDL_Color color);
		void drawMap(const MAP& map, int MAP_W, int MAP_H, DISPLAY_OPTIONS& options);
		void drawText(const char* text, int x, int y);
		void updateViewport(int w, int h);

		void applyZoom(int mouseX, int mouseY, int direction);

		void setOffset(int x, int y);

		int getOffsetX() const;
		int getOffsetY() const;
		int getScale() const;

		bool isValid() const;

		SDL_Renderer* getSDLRenderer() const;
		TTF_Font* getFont() const;
		/* Ressources SDL et paramètres internes de zoom/caméra. */
private:
		SDL_Renderer* sdlRenderer { nullptr };
		TTF_Font*     font        { nullptr };

		int scale   { 8 };
		int offsetX { 0 };
		int offsetY { 0 };

		/*
		* Internal drawing helpers.
		*/
		unsigned int hashCell(int x, int y) const;
};


