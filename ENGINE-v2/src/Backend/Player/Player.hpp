#ifndef PLAYER_HPP
#define PLAYER_HPP



class Player
{
	public:

		Player();
		Player(int i);
		virtual ~Player();

		const char *getName() const;

	private:

		int id;
		const char *name;
		int gold;
		int food;
		int foodCap;



	protected:

};


#endif