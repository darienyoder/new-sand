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
	FIRE,
	SMOKE,
	WOOD,
	AERIAL,
	FIREWORK,
};

class Particle
{
public:
	Particle() { material = EMPTY; x = 0; y = 0; };
	Particle(int m, SandSim& s) { material = m; sim = &s; };
	virtual ~Particle() {};

	void swap(int x_, int y_);
	virtual void move_to(int x_, int y_) { swap(x_, y_); };

	virtual bool tick() { return false; };

	void set_tile(int x_, int y_, int m_);
	void remove() { about_to_delete = true; };

	int last_tick = 0;

	int material = EMPTY;
	int x = 0, y = 0;
	int r = 255, g = 0, b = 0;
	int hp = 1;

	bool about_to_delete = false;

	SandSim* sim = 0;
};

extern Particle* sample[15];

class Air : virtual public Particle {};

class Powder : virtual public Particle
{
public:
	int density = 1001;
	bool tick();
	int get_move_speed(int x_, int y_);
};

class Solid : virtual public Particle
{
public:
	virtual void melt() {};
};

class Liquid : virtual public Particle
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

class Gas : virtual public Particle
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
		hp = 4;
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
	Ice()
	{
		hp = 3;
	}
	void melt() { set_tile(x, y, WATER); }
};

class Steam : public Gas
{
public:
	void condense() { set_tile(x, y, WATER); }
};

class Fire : public Gas
{
	bool tick();
};

class Smoke : public Gas
{

};

class Dirt : public Powder
{
public:
	Dirt()
	{
		density = 1600;
		hp = 5;
	}
};

class Stone : public Solid
{
public:
	Stone()
	{
		hp = 15;
	}
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

class Flammable : virtual public Particle
{
public:
	bool burning = false;
	bool tick();
};

class Oil : public Liquid, public Flammable
{
public:
	Oil()
	{
		density = 800;
	}
	bool tick();
};

class Wood : public Solid, public Flammable
{
public:
	Wood()
	{
		hp = 5;
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

enum {
	PARTICLE_MODE_GRAVITY,
	PARTICLE_MODE_DISTANCE,
	PARTICLE_MODE_TIME,
};

class Aerial : public Particle
{
public:
	Aerial() { material = AERIAL; };
	Aerial(Particle& source);

	bool tick();
	float speed() { return std::sqrtf(vel_x*vel_x + vel_y*vel_y); };

	void full_remove() { about_to_delete = true; about_to_full_delete = true; };

	float vel_x = 0;
	float vel_y = 0;

	int mode = 0;
	int age = 0;
	float dist_traveled = 0.0;

	float limit = 0.0;

	Particle* p = 0;

	bool about_to_full_delete = false;
};

class Firework : public Particle
{
public:
	Firework() {};
	bool tick();
};

#endif