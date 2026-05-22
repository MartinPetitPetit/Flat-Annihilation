#pragma  once



#include <string>
#include <vector>


#include "../Entity/Entity.hpp"
#include "../Resource/Resource.hpp"

class Player
{
	public:

		Player();
		Player(int i);
		virtual ~Player();

		const std::string getName() const;

	private:





	protected:

		int id;
		std::string name;
		//std::vector<Resource> Resources;
		std::vector<Entity*> entities;



};
