#include "Resource.hpp"
#include <SDL2/SDL_render.h>

Resource::Resource(ResourceType type)
{
	this->type = type;
	
	if (type == wood)
	{
		int roll = std::rand() % 100;
		
		// selection aléatoire du sprite de l'arbre
		// this->texture = ???

		SDL_Surface* surface = IMG_Load("pixel-art-tree-icon.png"); 

        SDL_FreeSurface(surface);
	}
}

Resource::~Resource()
{
	SDL_FreeSurface(this->surface);
	// SDL_DestroyTexture(this->texture);
}


ResourceType Resource::getResourceType()
{
	return this->type;
}




void Resource::render(SDL_Renderer *renderer, SDL_Rect destination)
{
        // m_Destination.x = m_Position.x;
        // m_Destination.y = m_Position.y;

	this->texture = SDL_CreateTextureFromSurface(renderer, this->surface); 
	SDL_RenderCopy(renderer, this->texture, NULL, &destination);
	SDL_DestroyTexture(this->texture);
}