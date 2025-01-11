#ifndef PLAYER_CHARACTER
#define PLAYER_CHARACTER

#include "input.hpp"
#include "sandsim.hpp"

class Player
{
public:
	Player(SandSim& sim_) { sim = &sim_; };
	void update(float delta);

	float x = 0, y = 0;
	float vel_x = 0, vel_y = 0;

	bool is_on_floor = false;

	SandSim* sim = 0;
private:
};

#endif