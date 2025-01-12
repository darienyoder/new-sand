#ifndef PLAYER_CHARACTER
#define PLAYER_CHARACTER

#include "input.hpp"
#include "sandsim.hpp"

class Player
{
public:
	Player(SandSim& sim_) { sim = &sim_; };
	void update(float delta);

	bool can_fit(int x_, int y_);

	float x = 50, y = 50;
	float vel_x = 0, vel_y = 0;

	bool is_on_floor = false;

	int size[2] = {9, 21};

	SandSim* sim = 0;
private:
};

#endif