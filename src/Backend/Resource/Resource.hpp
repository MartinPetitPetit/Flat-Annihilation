#pragma once

// enum ResourceType {food, wood, stone, gold};



// class Resource
// {

//     public:
//         Resource();
//         virtual ~Resource();


		Resource(ResourceType type, int amount, int maxAmount);

        virtual ~Resource();

		void render(SDL_Renderer *renderer, SDL_Rect destination);

		ResourceType getResourceType();

    private:

        ResourceType type;
        int amount;
        int maxAmount;

		SDL_Surface *surface;
		SDL_Texture *texture;
        
//     protected:
// };