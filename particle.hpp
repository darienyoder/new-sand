#ifndef PARTICLE
#define PARTICLE

#include "sandsim.hpp"
#include "iostream"

class SandSim;

enum {
	EMPTY,
	SAND,
	WATER,
};

class Particle
{
public:
	Particle() { material = EMPTY; x = 0; y = 0; };
	Particle(int m, SandSim& s) { material = m; sim = &s; };
	~Particle() {};

	void swap(int x_, int y_);
	void move_to(int x_, int y_) { swap(x_, y_); };

	virtual bool tick() { return false; };

	int last_tick = 0;

	int material = EMPTY;
	int x = 0, y = 0;
	int r = 255, g = 0, b = 0;

	SandSim* sim = 0;
};

class Air : public Particle
{

};

class Fluid : public Particle
{
public:
	int bias = std::rand() % 1 == 0 ? -1 : 1;
	bool flow_directions[5] = { 0, 0, 0, 0, 0 };
	bool tick();
};

class Sand : public Fluid
{
public:
	Sand() { flow_directions[0] = 1; flow_directions[1] = 1; };
};

class Water : public Fluid
{
public:
	Water() { flow_directions[0] = 1; flow_directions[1] = 1; flow_directions[2] = 1; };
};

#endif