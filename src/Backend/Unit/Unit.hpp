#pragma once

#include "../Player/Player.hpp"
#include "../Entity/Entity.hpp"
#include "../Map/Map.hpp"
#include "../Resource/Resource.hpp"
#include "../../Frontend/Renderer/Renderer.hpp"


class Unit : Entity
{
	public:
		Unit(int type, int x, int y, Player player);
		virtual ~Unit();

		void moveTo(int x, int y,Map map);

		void gather(C_Resource *Resource);

		void update(float dt);

		void render(Renderer *Renderer);

		// const UnitStats *getStats();


	private:

		// UnitStats stats;

		C_Resource* gatherTarget;
		int carriedAmount;

		void resolveAttack(float dt);
		void resolveGather(float dt);


	protected:


};

// -vector<Vector2f> path
// -Entity* target
// -PathFinder* pathfinder
// +attackTarget(e Entity*) : void
// -followPath(dt) : void
