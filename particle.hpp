#ifndef PARTICLE
#define PARTICLE

#include "sandsim.hpp"
#include "iostream"

class SandSim;

enum {
	EMPTY,
	SAND,
	WATER,
	ICE,
	STEAM,
	DIRT,
	STONE,
	LAVA,
	OIL,
	ACID,
};

class Particle
{
public:
	Particle() { material = EMPTY; x = 0; y = 0; };
	Particle(int m, SandSim& s) { material = m; sim = &s; };
	~Particle() {};

	void swap(int x_, int y_);
	virtual void move_to(int x_, int y_) { swap(x_, y_); };

	virtual bool tick() { return false; };

	void set_tile(int x_, int y_, int m_);
	void remove() { about_to_delete = true; };

	int last_tick = 0;

	int material = EMPTY;
	int x = 0, y = 0;
	int r = 255, g = 0, b = 0;

	bool about_to_delete = false;

	SandSim* sim = 0;
};

class Air : public Particle {};

class Powder : public Particle
{
public:
	int density = 1001;
	bool tick();
	int get_move_speed(int x_, int y_);
};

class Solid : public Particle
{
public:
	virtual void melt() {};
};

class Liquid : public Particle
{
public:
	int direction = std::rand() % 2 ? -1 : 1;
	int density = 1000; // Measured in kg/m^3; water is 1000
	int spread = 4;
	
	bool tick();
	virtual bool can_move_through(int x_, int y_);

	virtual void freeze() {};
	virtual void evaporate() {};
};

class Gas : public Particle
{
public:
	bool tick();
	virtual void condense() {};
};

class Sand : public Powder
{
public:
	Sand()
	{
		density = 1442;
	}
};

class Water : public Liquid
{
public:
	void freeze() { set_tile(x, y, ICE); }
	void evaporate() { set_tile(x, y, STEAM); }
};

class Ice : public Solid
{
public:
	void melt() { set_tile(x, y, WATER); }
};

class Steam : public Gas
{
public:
	void condense() { set_tile(x, y, WATER); }
};

class Dirt : public Powder
{
public:
	Dirt()
	{
		density = 1600;
	}
};

class Stone : public Solid
{

};

class Lava : public Liquid
{
public:
	Lava()
	{
		spread = 1;
		density = 1300;
	}
	bool tick();
};

class Oil : public Liquid
{
public:
	Oil()
	{
		density = 800;
	}
};

class Acid : public Liquid
{
public:
	Acid()
	{
		density = 400;
	}
	void move_to(int x_, int y_);
	bool can_move_through(int x_, int y_);
};

#endif