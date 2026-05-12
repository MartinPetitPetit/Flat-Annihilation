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





	protected:

		int id;
		const char *name;


};


#endif