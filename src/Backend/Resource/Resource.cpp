#include "Resource.hpp"


Resource::Resource(ResourceType type, int amount, int maxAmount)
{
	this->amount = amount;
	this->maxAmount = maxAmount;
	this->type = type;
	
	if (type == wood)
	{
		int roll = std::rand() % 100;
		
		// selection aléatoire du sprite de l'arbre
		// this->texture = ???

		this->surface = IMG_Load("assets/oak_tree.png"); 

        // SDL_FreeSurface(surface);
	}
	else if (type == food) {
		if (this->amount > 0) this->surface = IMG_Load("assets/berry.png");
		else this->surface = IMG_Load("assets/bush.png"); 
	}
}

Resource::~Resource()
{
	SDL_FreeSurface(this->surface);
	SDL_DestroyTexture(this->texture);
}


ResourceType Resource::getResourceType()
{
	return this->type;
}

void Resource::render(SDL_Renderer *renderer, SDL_Rect destination)
{
	if (this->texture == nullptr) this->texture = SDL_CreateTextureFromSurface(renderer, this->surface); 
	SDL_RenderCopy(renderer, this->texture, NULL, &destination);
}